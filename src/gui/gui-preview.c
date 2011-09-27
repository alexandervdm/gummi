/**
 * @file    gui-preview.c
 * @brief  
 *
 * Copyright (C) 2010-2011 Gummi-Dev Team <alexvandermey@gmail.com>
 * All Rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "gui/gui-preview.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include <poppler.h> 
#include <math.h>

#include "configfile.h"
#include "gui/gui-main.h"
#include "environment.h"

#include "motion.h"
#include "porting.h"



/* set up uri using appropriate formatting for OS 
   http://en.wikipedia.org/wiki/File_URI_scheme#Linux */
#ifdef WIN32
    const gchar *urifrmt = "file:///";
    gint usize = 8;
#else
    const gchar *urifrmt = "file://";
    gint usize = 7;
#endif

static enum {
    ZOOM_FIT_BOTH, 
    ZOOM_FIT_WIDTH, 
    ZOOM_50, 
    ZOOM_70, 
    ZOOM_85, 
    ZOOM_100, 
    ZOOM_125, 
    ZOOM_150, 
    ZOOM_200, 
    ZOOM_300, 
    ZOOM_400, 
    N_ZOOM_SIZES
};

static gfloat list_sizes[] = {-1, -1, 0.50, 0.70, 0.85, 1.0, 1.25, 1.5, 2.0,
                              3.0, 4.0};

extern Gummi* gummi;
extern GummiGui* gui;


static void previewgui_set_scale(GuPreviewGui* pc, gdouble scale, gdouble x, gdouble y);


/* Update functions, to update cached values and gui-parameters after changes */
static void update_scaled_size(GuPreviewGui* pc);
static void update_fit_scale(GuPreviewGui* pc);
static void update_current_page(GuPreviewGui* pc);
static void update_drawarea_size(GuPreviewGui *pc);
static void update_page_sizes(GuPreviewGui* pc);
static void update_prev_next_page(GuPreviewGui* pc);

/* Simplicity functions for page layout */
inline static gboolean is_continuous(GuPreviewGui* pc);

/* Functions for infos about the GUI */
inline static gboolean is_vscrollbar_visible(GuPreviewGui* pc);
inline static gboolean is_hscrollbar_visible(GuPreviewGui* pc);

/* Functions for simpler accessing of the array and struct data */
inline static gdouble get_page_height(GuPreviewGui* pc, int page);
inline static gdouble get_page_width(GuPreviewGui* pc, int page);

inline static gint get_document_margin(GuPreviewGui* pc);
inline static gint get_page_margin(GuPreviewGui* pc);

/* Other functions */
static void block_handlers_current_page(GuPreviewGui* pc);
static void unblock_handlers_current_page(GuPreviewGui* pc);
static void set_fit_mode(GuPreviewGui* pc, enum GuPreviewFitMode fit_mode);

/* Functions for layout and painting */
static gint page_offset_x(GuPreviewGui* pc, gint page, gdouble x);
static gint page_offset_y(GuPreviewGui* pc, gint page, gdouble x);
static void paint_page(cairo_t *cr, GuPreviewGui* pc, 
                       gint page, gint x, gint y);
static cairo_surface_t* get_page_rendering(GuPreviewGui* pc, int page);


GuPreviewGui* previewgui_init (GtkBuilder * builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuPreviewGui* p = g_new0 (GuPreviewGui, 1);
    GdkColor bg = {0, 0xed00, 0xec00, 0xeb00};
    p->previewgui_viewport =
        GTK_VIEWPORT (gtk_builder_get_object (builder, "preview_vport"));
    p->statuslight =
        GTK_WIDGET (gtk_builder_get_object (builder, "tool_statuslight"));
    p->drawarea =
        GTK_WIDGET (gtk_builder_get_object (builder, "preview_draw"));
    p->scrollw =
        GTK_WIDGET (gtk_builder_get_object (builder, "previewgui_scroll"));
    p->combo_sizes =
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "combo_sizes"));
    p->page_next = GTK_WIDGET (gtk_builder_get_object (builder, "page_next"));
    p->page_prev = GTK_WIDGET (gtk_builder_get_object (builder, "page_prev"));
    p->page_label = GTK_WIDGET (gtk_builder_get_object (builder, "page_label"));
    p->page_input = GTK_WIDGET (gtk_builder_get_object (builder, "page_input"));
    p->uri = NULL;
    p->doc = NULL;
    
    p->page_layout_single_page = GTK_RADIO_MENU_ITEM(gtk_builder_get_object (builder, "page_layout_single_page"));
    p->page_layout_one_column = GTK_RADIO_MENU_ITEM(gtk_builder_get_object (builder, "page_layout_one_column"));
    p->update_timer = 0;
    p->preview_on_idle = FALSE;
    p->hadj = gtk_scrolled_window_get_hadjustment
                    (GTK_SCROLLED_WINDOW (p->scrollw));
    p->vadj = gtk_scrolled_window_get_vadjustment
                    (GTK_SCROLLED_WINDOW (p->scrollw));

    gtk_widget_modify_bg (p->drawarea, GTK_STATE_NORMAL, &bg); 

    /* Install event handlers */
    gtk_widget_add_events (p->drawarea, GDK_SCROLL_MASK
                                      | GDK_BUTTON_PRESS_MASK
                                      | GDK_BUTTON_MOTION_MASK);

    p->page_input_changed_handler = g_signal_connect (p->page_input, 
            "changed", G_CALLBACK (on_page_input_changed), p);
    p->combo_sizes_changed_handler = g_signal_connect (p->combo_sizes, 
            "changed", G_CALLBACK (on_combo_sizes_changed), p);
    g_signal_connect (p->page_prev, 
            "clicked", G_CALLBACK (on_prev_page_clicked), p);
    g_signal_connect (p->page_next, 
            "clicked", G_CALLBACK (on_next_page_clicked), p);

    p->on_resize_handler = g_signal_connect (p->scrollw, "size-allocate", G_CALLBACK (on_resize), p);
    g_signal_connect (p->drawarea, "scroll-event", G_CALLBACK (on_scroll), p);
    p->on_expose_handler = g_signal_connect (p->drawarea, "expose-event", G_CALLBACK (on_expose), p);
    
    g_signal_connect (p->drawarea, "button-press-event",
                      G_CALLBACK (on_key_press), p);
    g_signal_connect (p->drawarea, "motion-notify-event",
                      G_CALLBACK (on_motion), p);
                      
    p->hvalue_changed_handler = g_signal_connect (p->hadj, "value-changed",
                      G_CALLBACK (on_adj_changed), p);
    p->vvalue_changed_handler = g_signal_connect (p->vadj, "value-changed",
                      G_CALLBACK (on_adj_changed), p);
    p->hchanged_handler = g_signal_connect (p->hadj, "changed",
                      G_CALLBACK (on_adj_changed), p);
    p->vchanged_handler = g_signal_connect (p->vadj, "changed",
                      G_CALLBACK (on_adj_changed), p);
                      
    
    /* The error panel is now imported from Glade. The following
     * functions re-parent the panel widgets for use in Gummi */
    GtkWidget *holder = 
        GTK_WIDGET(gtk_builder_get_object (builder, "errorwindow"));
    p->errorpanel = 
        GTK_WIDGET(gtk_builder_get_object (builder, "errorpanel"));

    gtk_container_remove(GTK_CONTAINER(holder), p->errorpanel);
    g_object_unref(holder);
    
    // The scale to correct for the users DPI
    gdouble poppler_scale = gdk_screen_get_resolution(gdk_screen_get_default())/72.;
    int i;
    for (i=0; i<N_ZOOM_SIZES; i++) {
        list_sizes[i] *= poppler_scale;
    }
    
    p->fit_mode = FIT_NONE;
    
    if (strcmp (config_get_value ("pagelayout"), "single_page") == 0) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(p->page_layout_single_page), TRUE);
        p->pageLayout = POPPLER_PAGE_LAYOUT_SINGLE_PAGE;
    } else  {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(p->page_layout_one_column), TRUE);
        p->pageLayout = POPPLER_PAGE_LAYOUT_ONE_COLUMN;
    }
    
    slog (L_INFO, "using libpoppler %s\n", poppler_get_version ());
    return p;
}

inline static gint get_document_margin(GuPreviewGui* pc) {
    if (pc->pageLayout == POPPLER_PAGE_LAYOUT_SINGLE_PAGE) {
        return 0;
    } else {
        return DOCUMENT_MARGIN;
    }
}

inline static gint get_page_margin(GuPreviewGui* pc) {
    return PAGE_MARGIN;
}

static void block_handlers_current_page(GuPreviewGui* pc) {
    
    g_signal_handler_block(pc->hadj, pc->hvalue_changed_handler);
    g_signal_handler_block(pc->vadj, pc->vvalue_changed_handler);
    g_signal_handler_block(pc->hadj, pc->hchanged_handler);
    g_signal_handler_block(pc->vadj, pc->vchanged_handler);
}

static void unblock_handlers_current_page(GuPreviewGui* pc) {
    
    g_signal_handler_unblock(pc->hadj, pc->hvalue_changed_handler);
    g_signal_handler_unblock(pc->vadj, pc->vvalue_changed_handler);
    g_signal_handler_unblock(pc->hadj, pc->hchanged_handler);
    g_signal_handler_unblock(pc->vadj, pc->vchanged_handler);
}

inline static gboolean is_vscrollbar_visible(GuPreviewGui* pc) {
    return pc->scrollw->allocation.width != GTK_WIDGET(pc->previewgui_viewport)->allocation.width;
}

inline static gboolean is_hscrollbar_visible(GuPreviewGui* pc) {
    return pc->scrollw->allocation.height != GTK_WIDGET(pc->previewgui_viewport)->allocation.height;
}

G_MODULE_EXPORT
void previewgui_page_layout_radio_changed(GtkMenuItem *radioitem, gpointer data) {
    L_F_DEBUG;
    
    if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(radioitem))) {
        return;
    }
    
    GuPreviewGui* pc = gui->previewgui;
    
    PopplerPageLayout pageLayout;
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(pc->page_layout_single_page))) {
        pageLayout = POPPLER_PAGE_LAYOUT_SINGLE_PAGE;
        config_set_value("pagelayout", "single_page");
    } else {
        pageLayout = POPPLER_PAGE_LAYOUT_ONE_COLUMN;
        config_set_value("pagelayout", "one_column");
    }
    
    previewgui_set_page_layout(gui->previewgui, pageLayout);
}

static gboolean previewgui_animated_scroll_step(gpointer data) {
    L_F_DEBUG;
    GuPreviewGui* pc = GU_PREVIEW_GUI(data);
    
    if (pc->ascroll_steps_left == 0) {
    
        return FALSE;
    } else if (pc->ascroll_steps_left == 1) {
    
        block_handlers_current_page(pc);
        previewgui_goto_xy (pc, pc->ascroll_end_x, pc->ascroll_end_y);
        unblock_handlers_current_page(pc);
        return FALSE;
    } else {
    
        pc->ascroll_steps_left -= 1;
    
        gdouble r = (2.*pc->ascroll_steps_left) / ASCROLL_STEPS - 1;
        gdouble r2 = r*r;
    
        gdouble rel_dist = 0.5*(ASCROLL_CONST_A * r2 * r2 * r + ASCROLL_CONST_B * r2 * r + ASCROLL_CONST_C * r) + 0.5;
        gdouble new_x = pc->ascroll_end_x + pc->ascroll_dist_x*rel_dist;
        gdouble new_y = pc->ascroll_end_y + pc->ascroll_dist_y*rel_dist;
    
        block_handlers_current_page(pc);
        previewgui_goto_xy (pc, new_x, new_y);
        unblock_handlers_current_page(pc);
    
        return TRUE;
    }
}

void previewgui_update_statuslight (const gchar* type) {
    gdk_threads_enter ();
    gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON(gui->previewgui->statuslight),
           type);
    gdk_threads_leave ();
}

static void update_fit_scale(GuPreviewGui* pc) {

    if (pc->fit_mode == FIT_NONE) {
        return;
    }
    L_F_DEBUG;
    
    gdouble width_scaling;
    gdouble height_scaling;
    gdouble width_non_scaling;
    gdouble height_non_scaling;

    width_scaling = pc->width_pages;
    width_non_scaling = 2*get_document_margin(pc);
    
    if (is_continuous(pc)) {
        height_scaling = pc->max_page_height;
        height_non_scaling = 2*get_document_margin(pc);
    } else {
        height_scaling = get_page_height(pc, pc->current_page);
        height_non_scaling = 2*get_document_margin(pc);
    }
    
    gdouble full_height_scaling = pc->height_pages;
    gdouble full_height_non_scaling = (pc->n_pages-1) * get_page_margin(pc) + 2*get_document_margin(pc);
    
    gint spacing;
    GtkRequisition req;
    gtk_widget_style_get (pc->scrollw, "scrollbar_spacing", &spacing, NULL);
    // Use gtk_widget_get_preferred_size with GTK+3.0
    gtk_widget_size_request(gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(pc->scrollw)), &req);
    gint vscrollbar_width = spacing + req.width;
    gtk_widget_size_request(gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(pc->scrollw)), &req);
    gint hscrollbar_height = spacing + req.height;
    
    gint view_width_without_bar = gdk_window_get_width(GDK_WINDOW(gtk_viewport_get_view_window(pc->previewgui_viewport)));
    if (gtk_widget_get_visible(gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(pc->scrollw)))) {
        view_width_without_bar += vscrollbar_width;
    }
    
    gint view_height_without_bar = gdk_window_get_height(GDK_WINDOW(gtk_viewport_get_view_window(pc->previewgui_viewport)));
    if (gtk_widget_get_visible(gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(pc->scrollw)))) {
        view_height_without_bar += hscrollbar_height;
    }
    gint view_width_with_bar = view_width_without_bar - vscrollbar_width;
    gint view_height_with_bar = view_height_without_bar - hscrollbar_height;
    
    gdouble scale_height_without_bar = (view_height_without_bar - height_non_scaling) / height_scaling;
    gdouble scale_full_height_without_bar = (view_height_without_bar - full_height_non_scaling) / full_height_scaling;
    gdouble scale_width_without_bar = (view_width_without_bar - width_non_scaling)  / width_scaling;
    gdouble scale_height_with_bar = (view_height_with_bar - height_non_scaling) / height_scaling;
    gdouble scale_width_with_bar = (view_width_with_bar - width_non_scaling)  / width_scaling;
    gdouble scale_both = MIN(scale_width_without_bar, scale_height_without_bar);
    gdouble scale_both_full = MIN(scale_width_without_bar, scale_full_height_without_bar);
    
    // When the preview window size is shrunk, in FIT_WIDTH and FIT_HEIGHT there
    // is a point right after the scrollbar has disappeared, where the document
    // should must not be shrunk, because the height just fits. We catch this 
    // case here.
    gdouble scale_height = MAX(scale_height_with_bar, scale_both_full);
    gdouble scale_width = MAX(scale_width_with_bar, scale_both_full);
    
    // Now for the scale_both....
    // Check if we need a bar:
    if (scale_full_height_without_bar < scale_both) {
        // We need a vsbar
        scale_both = MAX(scale_both_full, MIN(scale_width_with_bar, scale_height_without_bar));
    } else {
        // We do not need a vsbar, everything is fine...
    }
    
    gdouble scale = pc->scale;
    
    if (pc->fit_mode == FIT_WIDTH) {
        scale = scale_width;
    } else if (pc->fit_mode == FIT_HEIGHT) {
        scale = scale_height;
    } else if (pc->fit_mode == FIT_BOTH) {
        scale = scale_both;
    }
    
    if (scale == pc->scale) {
        return;
    }
    
    slog(L_DEBUG, "Document size wrong for fitting, changing scale from %f to %f.\n", pc->scale, scale);
    
    previewgui_set_scale(pc, scale, 
        gtk_adjustment_get_page_size(pc->hadj)/2, 
        gtk_adjustment_get_page_size(pc->vadj)/2);
}

inline static gboolean is_continuous(GuPreviewGui* pc) {

    if (pc->pageLayout == POPPLER_PAGE_LAYOUT_ONE_COLUMN) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static gint page_offset_x(GuPreviewGui* pc, gint page, gdouble x) {
    if (page < 0 || page >= pc->n_pages) {
        return 0;
    }
    
    return x + (pc->width_scaled - get_page_width(pc, page)*pc->scale) / 2;
}

static gint page_offset_y(GuPreviewGui* pc, gint page, gdouble y) {
    if (page < 0 || page >= pc->n_pages) {
        return 0;
    }
    
    return y;
}

static void previewgui_set_current_page(GuPreviewGui* pc, gint page) {
    
    page = MAX(0, page);
    page = MIN(page, pc->n_pages-1);
    
    // Always run the code below, in case the document has changed
    //if (pc->current_page == page) {
    //    return;
    //}
    L_F_DEBUG;
    
    pc->current_page = page;
    
    gchar* num = g_strdup_printf ("%d", page+1);
    g_signal_handler_block(pc->page_input, pc->page_input_changed_handler);
    gtk_entry_set_text (GTK_ENTRY(pc->page_input), num);
    g_signal_handler_unblock(pc->page_input, pc->page_input_changed_handler);
    g_free (num);
    
    update_prev_next_page(pc);
    
}

static void update_prev_next_page(GuPreviewGui* pc) {

    pc->next_page = pc->current_page + 1;
    if (pc->next_page >= pc->n_pages) {
        pc->next_page = -1;
    }
    pc->prev_page = pc->current_page - 1;
    if (pc->prev_page < 0) {
        pc->prev_page = -1;
    }

    gtk_widget_set_sensitive(pc->page_prev, (pc->prev_page != -1));
    gtk_widget_set_sensitive(pc->page_next, (pc->next_page != -1));
}

static void update_current_page(GuPreviewGui* pc) {

    // Only update current page when in continuous layout...
    if (!is_continuous(pc)) {
        return;
    }
    L_F_DEBUG;

    gdouble offset_y = MAX(get_document_margin(pc), (gtk_adjustment_get_page_size(pc->vadj) - pc->height_scaled)/2 );

    // TODO: This can be simplified...

    // The page margins are just for safety...
    gdouble view_start_y = gtk_adjustment_get_value(pc->vadj) - get_page_margin(pc);
    gdouble view_end_y   = view_start_y + gtk_adjustment_get_page_size(pc->vadj) + 2*get_page_margin(pc);
    
    gint page;
    for (page=0; page < pc->n_pages; page++) {
        offset_y += get_page_height(pc, page)*pc->scale + get_page_margin(pc);
        if (offset_y >= view_start_y) {
            break;
        }
    }
    
    // If the first page that is painted covers at least half the screen, 
    // it is the current one, otherwise it is the one after that.
    if (offset_y <= (view_start_y+view_end_y)/2)  {
        page += 1;
    }

    previewgui_set_current_page(pc, page);

}

inline static gdouble get_page_height(GuPreviewGui* pc, int page) {
    if (page < 0 || page >= pc->n_pages) {
        return -1;
    }
    return (pc->pages + page)->height;
}

inline static gdouble get_page_width(GuPreviewGui* pc, int page) {
    if (page < 0 || page >= pc->n_pages) {
        return -1;
    }
    return (pc->pages + page)->width;
}

static void previewgui_invalidate_renderings(GuPreviewGui* pc) {
    L_F_DEBUG;
    
    int i;
    for (i = 0; i < pc->n_pages; i++) {
        cairo_surface_destroy((pc->pages + i)->rendering);
        (pc->pages + i)->rendering = FALSE;
    }
    
}

static void update_drawarea_size(GuPreviewGui *pc) {
    L_F_DEBUG;

    gint width = 1;
    gint height = 1;
    
    // If the document should be fit, we set the requested size to 1 so 
    // scrollbars will not appear.
    switch (pc->fit_mode) {
        case FIT_NONE:
            width = pc->width_scaled + 2*get_document_margin(pc);
            height = pc->height_scaled + 2*get_document_margin(pc);
            break;
        case FIT_WIDTH:
            height = pc->height_scaled + 2*get_document_margin(pc);
            break;
        case FIT_HEIGHT:
            width = pc->width_scaled + 2*get_document_margin(pc);
            break;
        case FIT_BOTH:
            if (is_continuous(pc)) {
                height = pc->height_scaled + 2*get_document_margin(pc);
            }
            break;
    }
    
    gtk_widget_set_size_request (pc->drawarea, width, height);
    
    // The upper values probably get updated through signals, but in some cases
    // this is too slow, so we do it here manually...
    
    // Minimize the number of calls to on_adjustment_changed
    block_handlers_current_page(pc);
    gtk_adjustment_set_upper(pc->hadj, (width==1) ? gtk_adjustment_get_page_size(pc->hadj) : width);
    gtk_adjustment_set_upper(pc->vadj, (height==1) ? gtk_adjustment_get_page_size(pc->vadj) : height);
    gtk_adjustment_changed(pc->hadj);
    unblock_handlers_current_page(pc);
    gtk_adjustment_changed(pc->vadj);
    
}

static void update_page_sizes(GuPreviewGui* pc) {

    // recalculate document properties
    
    int i;
    // calculate document height and width
        pc->height_pages = 0;
        for (i=0; i < pc->n_pages; i++) {
            pc->height_pages += get_page_height(pc, i);
        }
        
        pc->width_pages = 0;
        for (i=0; i < pc->n_pages; i++) {
            pc->width_pages = MAX(pc->width_pages, get_page_width(pc, i));
        }
        
        pc->width_no_scale = pc->width_pages;
    
    pc->max_page_height = 0;
    for (i=0; i < pc->n_pages; i++) {
        pc->max_page_height = MAX(pc->max_page_height, get_page_height(pc, i));
    }
    
    update_scaled_size(pc);
    update_drawarea_size(pc);
    
    update_fit_scale(pc);
}

void previewgui_set_page_layout(GuPreviewGui* pc, PopplerPageLayout pageLayout) {
    L_F_DEBUG;
    
    if (pageLayout == POPPLER_PAGE_LAYOUT_UNSET) {
        return;
    }
    
    pc->pageLayout = pageLayout;
    
    update_page_sizes(pc);
    previewgui_goto_page(pc, pc->current_page);
}

static void set_fit_mode(GuPreviewGui* pc, enum GuPreviewFitMode fit_mode) {

    if (pc->fit_mode == fit_mode) {
        return;
    }
    L_F_DEBUG;
    
    pc->fit_mode = fit_mode;
    
    switch (fit_mode) {
        case FIT_NONE:
            config_set_value("zoommode", "nofit");
            break;
        case FIT_WIDTH:
            config_set_value("zoommode", "pagewidth");
            break;
        case FIT_HEIGHT:
            config_set_value("zoommode", "pageheight");
            break;
        case FIT_BOTH:
            config_set_value("zoommode", "bestfit");
            break;
    }
    
    update_fit_scale(pc);
}

static void update_scaled_size(GuPreviewGui* pc) {
    L_F_DEBUG;

    if (is_continuous(pc)) {
        pc->height_scaled = pc->height_pages*pc->scale + (pc->n_pages-1) * get_page_margin(pc);
    } else {
        pc->height_scaled = get_page_height(pc, pc->current_page) * pc->scale;
    }
    
    pc->width_scaled = pc->width_pages*pc->scale;
}

static void previewgui_set_scale(GuPreviewGui* pc, gdouble scale, gdouble x, gdouble y) {

    if (pc->scale == scale) {
        return;
    }
    L_F_DEBUG;
    
    slog(L_DEBUG, "Changing scale\n");
    
    gdouble old_x = (gtk_adjustment_get_value(pc->hadj) + x) / 
            (pc->width_scaled + 2*get_document_margin(pc));
    gdouble old_y = (gtk_adjustment_get_value(pc->vadj) + y) / 
            (pc->height_scaled + 2*get_document_margin(pc));
            
    pc->scale = scale;
    
    update_scaled_size(pc);
    
    previewgui_invalidate_renderings(pc);
    
    // TODO: Blocking the expose event is porbably not the best way.
    // It would be great if we could change all 3 porperties (hadj, vadj & scale)
    // at the same time.
    // Probably blocking the expose handler causes the gray background of the
    // window to be drawn - but at least we do not scroll to a different page
    // anymore...
    // Without blocking the handler, after changing the first property, e.g.
    // vadj, a signal is emitted that causes a redraw but still contains the
    // the not-updated hadj & scale values.
    g_signal_handler_block(pc->drawarea, pc->on_expose_handler);
    
    update_drawarea_size(pc);
    
    if (x >= 0 && y>= 0) {
        gdouble new_x = old_x * (pc->width_scaled + 2*get_document_margin(pc)) - x;
        gdouble new_y = old_y * (pc->height_scaled + 2*get_document_margin(pc)) - y;

        previewgui_goto_xy(pc, new_x, new_y);
    }
    g_signal_handler_unblock(pc->drawarea, pc->on_expose_handler);
    
    gtk_widget_queue_draw (pc->drawarea);
    
    
        
}

static void previewgui_load_document(GuPreviewGui* pc, gboolean update) {
    L_F_DEBUG;
    
    previewgui_invalidate_renderings(pc);
    g_free(pc->pages);

    pc->n_pages = poppler_document_get_n_pages (pc->doc);
    gtk_label_set_text (GTK_LABEL (pc->page_label), 
            g_strdup_printf (_("of %d"), pc->n_pages));
    
    pc->pages = g_new0(GuPreviewPage, pc->n_pages);
        
    int i;
    for (i=0; i < pc->n_pages; i++) {
        PopplerPage *poppler = poppler_document_get_page(pc->doc, i);
        
        GuPreviewPage *page = pc->pages + i;
        poppler_page_get_size(poppler, &(page->width), &(page->height));
        g_object_unref(poppler);
        poppler = NULL;
    }
    
    update_page_sizes(pc);
    update_prev_next_page(pc);
}

void previewgui_set_pdffile (GuPreviewGui* pc, const gchar *pdffile) {
    L_F_DEBUG;
    previewgui_cleanup_fds (pc);

    pc->uri = g_strconcat (urifrmt, pdffile, NULL);

    pc->doc = poppler_document_new_from_file (pc->uri, NULL, NULL);
    g_return_if_fail (pc->doc != NULL);
    
    pc->restore_x = -1;
    pc->restore_y = -1;

    previewgui_load_document(pc, FALSE);    
    
    // Restore scale and fit mode
    g_signal_handler_block(pc->combo_sizes, pc->combo_sizes_changed_handler);
    if (strcmp (config_get_value ("zoommode"), "pagewidth") == 0) {
        set_fit_mode(pc, FIT_WIDTH);
        gtk_combo_box_set_active(pc->combo_sizes, ZOOM_FIT_WIDTH);
    } else if (strcmp (config_get_value ("zoommode"), "pageheight") == 0) {
        set_fit_mode(pc, FIT_HEIGHT);
        //gtk_combo_box_set_active(pc->combo_sizes, new_index);
    } else if (strcmp (config_get_value ("zoommode"), "bestfit") == 0) {
        set_fit_mode(pc, FIT_BOTH);
        gtk_combo_box_set_active(pc->combo_sizes, ZOOM_FIT_BOTH);
    } else {
        set_fit_mode(pc, FIT_NONE);
        previewgui_set_scale(pc, list_sizes[ZOOM_100], 
                gtk_adjustment_get_page_size(pc->hadj)/2, 
                gtk_adjustment_get_page_size(pc->vadj)/2);
        gtk_combo_box_set_active(pc->combo_sizes, ZOOM_100);
    }
    g_signal_handler_unblock(pc->combo_sizes, pc->combo_sizes_changed_handler);
    
    
    previewgui_goto_page (pc, 0);
}

void previewgui_refresh (GuPreviewGui* pc) {
    L_F_DEBUG;

    /* We lock the mutex to prevent previewing imcomplete PDF file, i.e
     * compiling. Also prevent PDF from changing (compiling) when previewing */
    if (!g_mutex_trylock (gummi->motion->compile_mutex)) return;

    /* This line is very important, if no pdf exist, preview will fail */
    if (!pc->uri || !utils_path_exists (pc->uri + usize)) goto unlock;

    previewgui_cleanup_fds (pc);

    pc->doc = poppler_document_new_from_file (pc->uri, NULL, NULL);

    /* release mutex and return when poppler doc is damaged or missing */
    if (pc->doc == NULL) goto unlock;

    previewgui_load_document(pc, TRUE);

    gtk_widget_queue_draw (pc->drawarea);

unlock:
    g_mutex_unlock (gummi->motion->compile_mutex);
}

void previewgui_goto_page (GuPreviewGui* pc, int page) {
    L_F_DEBUG;
    page = MAX(page, 0);
    page = MIN(page, pc->n_pages-1);
    
    previewgui_set_current_page(pc, page);
    
    gint i;
    gdouble x = 0;
    gdouble y = 0;
    
    if (!is_continuous(pc)) {
        update_scaled_size(pc);
        update_drawarea_size(pc);
    } else {
        for (i=0; i < page; i++) {
            y += get_page_height(pc, i)*pc->scale + get_page_margin(pc);
        }
    }
    
    previewgui_goto_xy(pc, page_offset_x(pc, page, x), 
                           page_offset_y(pc, page, y));
                           
    if (!is_continuous(pc)) {
        gtk_widget_queue_draw (pc->drawarea);
    }
}

void previewgui_scroll_to_page (GuPreviewGui* pc, int page) {
    L_F_DEBUG;
    
    if (!is_continuous(pc)) {
        // We do not scroll in single page mode...
        previewgui_goto_page(pc, page);
        return;
    }
    
    page = MAX(page, 0);
    page = MIN(page, pc->n_pages-1);
    
    previewgui_set_current_page(pc, page);
    
    gint i;
    gdouble x = 0;
    gdouble y = 0;
    for (i=0; i < page; i++) {
        y += get_page_height(pc, i)*pc->scale + get_page_margin(pc);
    }
    
    previewgui_scroll_to_xy(pc, page_offset_x(pc, page, x), 
                           page_offset_y(pc, page, y));
}

void previewgui_goto_xy (GuPreviewGui* pc, int x, int y) {
    L_F_DEBUG;

    GtkRequisition requisition;
    gtk_widget_size_request (pc->drawarea, &requisition);

    GdkWindow *w = gtk_viewport_get_view_window(pc->previewgui_viewport);
    gdouble page_x = gdk_window_get_width(w);
    gdouble page_y = gdk_window_get_height(w);

    gdouble upper_x = MAX(page_x, requisition.width);
    gdouble upper_y = MAX(page_y, requisition.height);

    x = MIN(x, upper_x - page_x);
    y = MIN(y, upper_y - page_y);
/*
    gtk_adjustment_configure(pc->hadj, x, 0, upper_x, 
            gtk_adjustment_get_step_increment(pc->hadj), 
            gtk_adjustment_get_page_increment(pc->hadj), 
            page_x);
    gtk_adjustment_configure(pc->vadj, y, 0, upper_y, 
            gtk_adjustment_get_step_increment(pc->vadj), 
            gtk_adjustment_get_page_increment(pc->vadj), 
            page_y);
    */
    
    // Minimize the number of calls to on_adjustment_changed
    block_handlers_current_page(pc);
    gtk_adjustment_set_value(pc->hadj, x);
    gtk_adjustment_set_value(pc->vadj, y);
    gtk_adjustment_value_changed(pc->hadj);
    unblock_handlers_current_page(pc);
    gtk_adjustment_value_changed(pc->vadj);
    
}

void previewgui_scroll_to_xy (GuPreviewGui* pc, int x, int y) {
    L_F_DEBUG;

    x = MIN(x, gtk_adjustment_get_upper(pc->hadj) - gtk_adjustment_get_page_size(pc->hadj));
    y = MIN(y, gtk_adjustment_get_upper(pc->vadj) - gtk_adjustment_get_page_size(pc->vadj));

    pc->ascroll_steps_left = ASCROLL_STEPS;
    
    pc->ascroll_end_x = x;
    pc->ascroll_end_y = y;
    
    pc->ascroll_dist_x = gtk_adjustment_get_value(pc->hadj) - x;
    pc->ascroll_dist_y = gtk_adjustment_get_value(pc->vadj) - y;
        
    g_timeout_add (1000./25., previewgui_animated_scroll_step, pc);
                                                         
}

void previewgui_save_position (GuPreviewGui* pc) {
    L_F_DEBUG;
    /* update last scroll position to restore it after error mode */
    pc->restore_y = gtk_adjustment_get_value(pc->vadj);
    pc->restore_x = gtk_adjustment_get_value(pc->hadj);
    block_handlers_current_page(pc);
    
    slog(L_DEBUG, "Position SAVED\n\n\n");
}

void previewgui_restore_position (GuPreviewGui* pc) {
    L_F_DEBUG;
    /* restore scroll window position to value before error mode */
    /* TODO: might want to merge this with synctex funcs in future */
    
    previewgui_goto_xy(pc, pc->restore_x, pc->restore_y);
    unblock_handlers_current_page(pc);
}

static cairo_surface_t* do_render(PopplerPage* ppage, gdouble scale, 
                                  gint width, gint height) {
    
    cairo_surface_t* r = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                width*scale, 
                                                height*scale);
    cairo_t *c = cairo_create(r);

    cairo_scale (c, scale, scale);
    poppler_page_render(ppage, c);

    // TODO for what is this used?
    cairo_set_operator (c, CAIRO_OPERATOR_DEST_OVER);
    cairo_set_source_rgb (c, 1, 1, 1);
    cairo_paint (c);
    cairo_destroy (c);
    
    return r;
}

static cairo_surface_t* get_page_rendering(GuPreviewGui* pc, int page) {

    GuPreviewPage *p = pc->pages + page;
    
    if (p->rendering == NULL) {
        PopplerPage* ppage = poppler_document_get_page(pc->doc, page);
        p->rendering = do_render(ppage, pc->scale, p->width, p->height);
        g_object_unref(ppage);
    }
    
    return cairo_surface_reference(p->rendering);
}

void previewgui_reset (GuPreviewGui* pc) {
    L_F_DEBUG;
    /* reset uri */
    g_free (pc->uri);
    pc->uri = NULL;

    gummi->latex->modified_since_compile = TRUE;
    previewgui_stop_preview (pc);
    motion_do_compile (gummi->motion);

    if (config_get_value ("compile_status"))
        previewgui_start_preview (pc);
}

void previewgui_quit(GuPreviewGui* pc) {
    
}

void previewgui_cleanup_fds (GuPreviewGui* pc) {
    L_F_DEBUG;

    if (pc->doc) {
        g_object_unref (pc->doc);
        pc->doc = NULL;
    }
}

void previewgui_start_preview (GuPreviewGui* pc) {
    if (0 == strcmp (config_get_value ("compile_scheme"), "on_idle")) {
        pc->preview_on_idle = TRUE;
    } else {
        pc->update_timer = g_timeout_add_seconds (
                atoi (config_get_value ("compile_timer")),
                motion_do_compile, gummi->motion);
    }
}

void previewgui_stop_preview (GuPreviewGui* pc) {
    pc->preview_on_idle = FALSE;
    if (pc->update_timer != 0)
        g_source_remove (pc->update_timer);
    pc->update_timer = 0;
}

G_MODULE_EXPORT
void on_page_input_changed (GtkEntry* entry, void* user) {
    L_F_DEBUG;

    gint newpage = atoi (gtk_entry_get_text (entry));
    newpage -= 1;
    newpage = MAX(newpage, 0);
    newpage = MIN(newpage, gui->previewgui->n_pages);
    slog(L_INFO, "page set to %i\n", newpage);
    previewgui_scroll_to_page (gui->previewgui, newpage);

}

G_MODULE_EXPORT
void on_next_page_clicked (GtkWidget* widget, void* user) {
    L_F_DEBUG;
    GuPreviewGui *pc = gui->previewgui;
    
    previewgui_scroll_to_page (pc, pc->next_page);
}

G_MODULE_EXPORT
void on_prev_page_clicked (GtkWidget* widget, void* user) {
    L_F_DEBUG;
    GuPreviewGui *pc = gui->previewgui;

    previewgui_scroll_to_page (pc, pc->prev_page);
}

G_MODULE_EXPORT
void on_combo_sizes_changed (GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint index = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    
    if (index == 0) {
        set_fit_mode(gui->previewgui, FIT_BOTH);
    } else if (index == 1) {
        set_fit_mode(gui->previewgui, FIT_WIDTH);
    } else {
        set_fit_mode(gui->previewgui, FIT_NONE);
        previewgui_set_scale(gui->previewgui, list_sizes[index], 
                gtk_adjustment_get_page_size(gui->previewgui->hadj)/2, 
                gtk_adjustment_get_page_size(gui->previewgui->vadj)/2);
    }
    
}

static void paint_page(cairo_t *cr, GuPreviewGui* pc, gint page, gint x, gint y) {
    if (page < 0 || page >= pc->n_pages) {
        return;
    }
    
    //slog (L_DEBUG, "printing page %i at (%i, %i)\n", page, x, y);
    
    gdouble page_width = get_page_width(pc, page) * pc->scale;
    gdouble page_height = get_page_height(pc, page) * pc->scale;

    // Paint shadow
    cairo_set_source_rgb (cr, 0.302, 0.302, 0.302);
    cairo_rectangle (cr, x + page_width , y + PAGE_SHADOW_OFFSET , PAGE_SHADOW_WIDTH, page_height);
    cairo_fill (cr);
    cairo_rectangle (cr, x + PAGE_SHADOW_OFFSET , y + page_height, 
                         page_width - PAGE_SHADOW_OFFSET, PAGE_SHADOW_WIDTH);
    cairo_fill (cr);

    // Paint border around page
    cairo_set_line_width (cr, 0.5);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_rectangle (cr, x - 1, y - 1, page_width + 1, page_height + 1);
    cairo_stroke (cr);
    
    cairo_surface_t* rendering = get_page_rendering(pc, page);
    
    // Paint rendering
    cairo_set_source_surface (cr, rendering, x, y);
    cairo_paint (cr);
    cairo_surface_destroy(rendering);
        
    
}

gboolean on_expose (GtkWidget* w, GdkEventExpose* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);

//    slog(L_INFO, "paint document with scale %f, region (%i, %i), w=%i, h=%i\n", e->area.x, e->area.y, e->area.width, e->area.height);

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) {
        
        return FALSE;
    }

    gdouble page_width = gtk_adjustment_get_page_size(pc->hadj);
    gdouble page_height = gtk_adjustment_get_page_size(pc->vadj);
        
    gdouble offset_x = MAX(get_document_margin(pc), (page_width - pc->width_scaled)/2 );
    
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(w));
    
    if (is_continuous(pc)) {
        
        gdouble offset_y = MAX(get_document_margin(pc), (page_height - pc->height_scaled)/2 );

        // The page margins are just for safety...
        gdouble view_start_y = gtk_adjustment_get_value(pc->vadj) - get_page_margin(pc);
        gdouble view_end_y   = view_start_y + page_height + 2*get_page_margin(pc);
        
        int i;
        for (i=0; i < pc->n_pages; i++) {
            offset_y += get_page_height(pc, i)*pc->scale + get_page_margin(pc);
            if (offset_y >= view_start_y) {
                break;
            }
        }
        
        // We added one offset to many...
        offset_y -= get_page_height(pc, i)*pc->scale + get_page_margin(pc);
        
        for (; i < pc->n_pages; i++) {
        
            paint_page(cr, pc, i, 
                page_offset_x(pc, i, offset_x),
                page_offset_y(pc, i, offset_y));
            
            offset_y += get_page_height(pc, i)*pc->scale + get_page_margin(pc);
            
            if (offset_y > view_end_y) {
                break;
            }
        }
        
    } else {    // "Page" Layout...
    
        gdouble height = get_page_height(pc, pc->current_page) * pc->scale;
        gdouble offset_y = MAX(get_document_margin(pc), (page_height - height)/2 );
        
        paint_page(cr, pc, pc->current_page, 
            page_offset_x(pc, pc->current_page, offset_x),
            page_offset_y(pc, pc->current_page, offset_y));
    }
    
    cairo_destroy (cr);
    
    return TRUE;
}

gboolean on_adj_changed(GtkAdjustment *adjustment, gpointer user) {
    L_F_DEBUG;
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    
    // Abort any animated scrolls that might be running...
    pc->ascroll_steps_left = 0;
    
    update_current_page(pc);
    
    return TRUE;
}

gboolean on_scroll (GtkWidget* w, GdkEventScroll* e, void* user) {
    L_F_DEBUG;
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    
    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;

    if (GDK_CONTROL_MASK & e->state) {
        
        gdouble old_scale = pc->scale;
        gdouble new_scale = -1;
        gint    new_index = -1;
        int i;
        
        // we only go through the percentage entrys - the fit entrys are not always
        // uo to date...
        for (i=0; i<N_ZOOM_SIZES; i++) {
            if (i == ZOOM_FIT_WIDTH || i == ZOOM_FIT_BOTH) {
                continue;
            }
            if (list_sizes[i] > old_scale && e->direction == GDK_SCROLL_UP) {
                if (new_index == -1 || list_sizes[i] < new_scale) {
                    new_scale = list_sizes[i];
                    new_index = i;
                }
            } else if (list_sizes[i] < old_scale && e->direction == GDK_SCROLL_DOWN) {
                if (new_index == -1 || list_sizes[i] > new_scale) {
                    new_scale = list_sizes[i];
                    new_index = i;
                }
            }
        }
        
        if (new_index != -1) {
    
            previewgui_set_scale(pc, list_sizes[new_index], 
                e->x - gtk_adjustment_get_value(pc->hadj), 
                e->y - gtk_adjustment_get_value(pc->vadj));
            
            set_fit_mode(pc, FIT_NONE);
            g_signal_handler_block(pc->combo_sizes, pc->combo_sizes_changed_handler);
            gtk_combo_box_set_active(pc->combo_sizes, new_index);
            g_signal_handler_unblock(pc->combo_sizes, pc->combo_sizes_changed_handler);
        
        }
            
        update_current_page(pc);
        
        return TRUE;
        
    } else if (e->state & GDK_SHIFT_MASK) { 
        // Shift+Wheel scrolls the in the perpendicular direction
        if (e->direction == GDK_SCROLL_UP)
            e->direction = GDK_SCROLL_LEFT;
        else if (e->direction == GDK_SCROLL_LEFT)
            e->direction = GDK_SCROLL_UP;
        else if (e->direction == GDK_SCROLL_DOWN)
            e->direction = GDK_SCROLL_RIGHT;
        else if (e->direction == GDK_SCROLL_RIGHT)
            e->direction = GDK_SCROLL_DOWN;

        e->state &= ~GDK_SHIFT_MASK;
    } else {
        // Scroll if no scroll bars visible
        
        if (!is_vscrollbar_visible(pc)) {
            gint tmp_page;
            switch (e->direction) {
                case GDK_SCROLL_UP:
                
                    if (pc->prev_page != -1) {
                        previewgui_scroll_to_page (pc, pc->prev_page);
                    }
                
                    break;
                case GDK_SCROLL_DOWN:
                
                    if (pc->next_page != -1) {
                        previewgui_scroll_to_page (pc, pc->next_page);
                    }
                    break;
                
                default:
                    // Do nothing
                    break;
            } 
            return TRUE;
        }
    }
    return FALSE;
}

gboolean on_key_press (GtkWidget* w, GdkEventButton* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    
    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;
    
    pc->prev_x = e->x;
    pc->prev_y = e->y;
    return FALSE;
}

gboolean on_motion (GtkWidget* w, GdkEventMotion* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    
    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;
    
    gdouble new_x = gtk_adjustment_get_value (pc->hadj) - (e->x - pc->prev_x);
    gdouble new_y = gtk_adjustment_get_value (pc->vadj) - (e->y - pc->prev_y);
    
    previewgui_goto_xy(pc, new_x, new_y);

    return TRUE;
}

gboolean on_resize (GtkWidget* w, GdkRectangle* r, void* user) {
    L_F_DEBUG;
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    
    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;
    
    update_fit_scale(pc);
    
    return FALSE;
}
