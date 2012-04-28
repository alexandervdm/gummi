/**
 * @file    gui-preview.c
 * @brief 
 *
 * Copyright (C) 2009-2012 Gummi-Dev Team <alexvandermey@gmail.com>
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
#include "constants.h"
#include "gui/gui-main.h"
#include "environment.h"

#include "motion.h"
#include "porting.h"

#include "syncTeX/synctex_parser.h"


/* set up uri using appropriate formatting for OS
   http://en.wikipedia.org/wiki/File_URI_scheme#Linux */
#ifdef WIN32
    const gchar *urifrmt = "file:///";
    gint usize = 8;
#else
    const gchar *urifrmt = "file://";
    gint usize = 7;
#endif

enum {
    ZOOM_FIT_BOTH = 0,
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

typedef struct {
    gint page;
    gint x; // of lower left corner
    gint y; // of lower left corner
    gint width;
    gint height;
    gint score;
} SyncNode;

static void previewgui_set_scale(GuPreviewGui* pc, gdouble scale, gdouble x,
                                 gdouble y);


/* Update functions, to update cached values and gui-parameters after changes */
static void update_scaled_size(GuPreviewGui* pc);
static void update_fit_scale(GuPreviewGui* pc);
static void update_current_page(GuPreviewGui* pc);
static void update_drawarea_size(GuPreviewGui *pc);
static void update_page_sizes(GuPreviewGui* pc);
static void update_prev_next_page(GuPreviewGui* pc);
static void update_page_input(GuPreviewGui* pc);

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

static gboolean on_page_input_lost_focus(GtkWidget *widget, GdkEvent  *event, 
                                         gpointer   user_data);
                                         
                                         
static gboolean on_button_pressed(GtkWidget* w, GdkEventButton* e, void* user);

/* Functions for layout and painting */
static gint page_offset_x(GuPreviewGui* pc, gint page, gdouble x);
static gint page_offset_y(GuPreviewGui* pc, gint page, gdouble x);
static void paint_page(cairo_t *cr, GuPreviewGui* pc,
                       gint page, gint x, gint y);
static cairo_surface_t* get_page_rendering(GuPreviewGui* pc, int page);
static gboolean remove_page_rendering(GuPreviewGui* pc, gint page);

/* Functions for scronizing editor and preview via SyncTeX */
static gboolean synctex_run_parser(GuPreviewGui* pc, GtkTextIter *sync_to,
                                   gchar* tex_file);
#if HAVE_POPPLER_PAGE_GET_SELECTED_TEXT
static void synctex_filter_results(GuPreviewGui* pc, GtkTextIter *sync_to);
#endif
static void synctex_scroll_to_node(GuPreviewGui* pc, SyncNode* node);
static SyncNode* synctex_one_node_found(GuPreviewGui* pc);
static void synctex_merge_nodes(GuPreviewGui* pc);
static void synctex_clear_sync_nodes(GuPreviewGui* pc);

/* (Page) Layout functions */
static inline LayeredRectangle get_fov(GuPreviewGui* pc);
static void update_page_positions(GuPreviewGui* pc);
static gboolean layered_rectangle_intersect(const LayeredRectangle *src1,
                                            const LayeredRectangle *src2,
                                            LayeredRectangle *dest);
//static gboolean layered_rectangle_union(const LayeredRectangle *src1,
//                                        const LayeredRectangle *src2,
//                                        LayeredRectangle *dest);
#define page_inner(pc,i) (((pc)->pages + (i))->inner)
#define page_outer(pc,i) (((pc)->pages + (i))->outer)

GuPreviewGui* previewgui_init (GtkBuilder * builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuPreviewGui* p = g_new0 (GuPreviewGui, 1);
    GdkColor bg = {0, 0xed00, 0xec00, 0xeb00};
    p->previewgui_viewport =
        GTK_VIEWPORT (gtk_builder_get_object (builder, "preview_vport"));
    p->previewgui_toolbar = 
        GTK_WIDGET (gtk_builder_get_object (builder, "previewgui_toolbar"));
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

    p->page_layout_single_page = GTK_RADIO_MENU_ITEM
        (gtk_builder_get_object (builder, "page_layout_single_page"));
    p->page_layout_one_column = GTK_RADIO_MENU_ITEM
        (gtk_builder_get_object (builder, "page_layout_one_column"));
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
    g_signal_connect (p->page_input,
            "focus-out-event", G_CALLBACK(on_page_input_lost_focus), p);
    p->combo_sizes_changed_handler = g_signal_connect (p->combo_sizes,
            "changed", G_CALLBACK (on_combo_sizes_changed), p);
    g_signal_connect (p->page_prev,
            "clicked", G_CALLBACK (on_prev_page_clicked), p);
    g_signal_connect (p->page_next,
            "clicked", G_CALLBACK (on_next_page_clicked), p);

    p->on_resize_handler = g_signal_connect (p->scrollw, "size-allocate",
            G_CALLBACK (on_resize), p);
    g_signal_connect (p->drawarea, "scroll-event", G_CALLBACK (on_scroll), p);
    p->on_expose_handler = g_signal_connect (p->drawarea, "expose-event",
            G_CALLBACK (on_expose), p);

    g_signal_connect (p->drawarea, "button-press-event",
                      G_CALLBACK (on_button_pressed), p);
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
    gdouble screen_dpi = gdk_screen_get_resolution (
						 gdk_screen_get_default());
            
	/* the gdk screen functions do not work properly on win32, so we 
	 * should probably return a default value (or find an alternative)*/
    if (screen_dpi == -1) screen_dpi = 96.0;
	gdouble poppler_scale = screen_dpi / 72.0;

    int i;
    for (i=0; i<N_ZOOM_SIZES; i++) {
        list_sizes[i] *= poppler_scale;
    }

    p->fit_mode = FIT_NONE;
    
    /* TODO: temporary measure because the config system does not look up
             default value if the setting is undefined */
    if (STR_EQU (config_get_value("zoommode"), "")) {
        config_set_value("zoommode", "pagewidth");
    }
    
    if (STR_EQU (config_get_value("animated_scroll"), "")) {
        config_set_value("animated_scroll", "always");
    }
    
    if (STR_EQU (config_get_value("cache_size"), "")) {
        config_set_value("cache_size", "150");
    }

    if (STR_EQU (config_get_value ("pagelayout"), "single_page")) {
        gtk_check_menu_item_set_active(
                GTK_CHECK_MENU_ITEM(p->page_layout_single_page), TRUE);
        p->pageLayout = POPPLER_PAGE_LAYOUT_SINGLE_PAGE;
    } else  {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                    p->page_layout_one_column), TRUE);
        p->pageLayout = POPPLER_PAGE_LAYOUT_ONE_COLUMN;
    }
    
    p->sync_nodes = NULL;

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
    return pc->scrollw->allocation.width !=
        GTK_WIDGET(pc->previewgui_viewport)->allocation.width;
}

inline static gboolean is_hscrollbar_visible(GuPreviewGui* pc) {
    return pc->scrollw->allocation.height !=
        GTK_WIDGET(pc->previewgui_viewport)->allocation.height;
}

G_MODULE_EXPORT
void previewgui_page_layout_radio_changed(GtkMenuItem *radioitem, gpointer data) {
    //L_F_DEBUG;

    if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM(radioitem))) {
        return;
    }

    GuPreviewGui* pc = gui->previewgui;

    PopplerPageLayout pageLayout;
    if (gtk_check_menu_item_get_active(
                GTK_CHECK_MENU_ITEM(pc->page_layout_single_page))) {
        pageLayout = POPPLER_PAGE_LAYOUT_SINGLE_PAGE;
        config_set_value("pagelayout", "single_page");
    } else {
        pageLayout = POPPLER_PAGE_LAYOUT_ONE_COLUMN;
        config_set_value("pagelayout", "one_column");
    }

    previewgui_set_page_layout(gui->previewgui, pageLayout);
}

static gboolean previewgui_animated_scroll_step(gpointer data) {
    //L_F_DEBUG;
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

        gdouble rel_dist = 0.5*(ASCROLL_CONST_A * r2 * r2 * r +
                ASCROLL_CONST_B * r2 * r + ASCROLL_CONST_C * r) + 0.5;
        gdouble new_x = pc->ascroll_end_x + pc->ascroll_dist_x*rel_dist;
        gdouble new_y = pc->ascroll_end_y + pc->ascroll_dist_y*rel_dist;

        block_handlers_current_page(pc);
        previewgui_goto_xy (pc, new_x, new_y);
        unblock_handlers_current_page(pc);

        return TRUE;
    }
}

void previewgui_update_statuslight (const gchar* type) {
    gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON(gui->previewgui->statuslight),
           type);
}

static void update_fit_scale(GuPreviewGui* pc) {

    if (pc->fit_mode == FIT_NONE) {
        return;
    }
    //L_F_DEBUG;

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
    gdouble full_height_non_scaling = (pc->n_pages-1) * get_page_margin(pc) +
        2*get_document_margin(pc);

    gint spacing;
    GtkRequisition req;
    gtk_widget_style_get (pc->scrollw, "scrollbar_spacing", &spacing, NULL);
    // Use gtk_widget_get_preferred_size with GTK+3.0
    gtk_widget_size_request(gtk_scrolled_window_get_vscrollbar(
                GTK_SCROLLED_WINDOW(pc->scrollw)), &req);
    gint vscrollbar_width = spacing + req.width;
    gtk_widget_size_request(gtk_scrolled_window_get_hscrollbar(
                GTK_SCROLLED_WINDOW(pc->scrollw)), &req);
    gint hscrollbar_height = spacing + req.height;


#if GTK_MINOR_VERSION >= 24 // gdk_window_get_width is gtk-2.24 or higher.
    gint view_width_without_bar = gdk_window_get_width(
            pc->previewgui_viewport->view_window);
    gint view_height_without_bar = gdk_window_get_height(
            pc->previewgui_viewport->view_window);
#else
        gint view_width_without_bar, view_height_without_bar;
        gdk_drawable_get_size (pc->previewgui_viewport->view_window, 
                                &view_width_without_bar, 
                                &view_height_without_bar);
#endif
    

    if (gtk_widget_get_visible(gtk_scrolled_window_get_vscrollbar(
                    GTK_SCROLLED_WINDOW(pc->scrollw)))) {
        view_width_without_bar += vscrollbar_width;
    }

    if (gtk_widget_get_visible(gtk_scrolled_window_get_hscrollbar(
                    GTK_SCROLLED_WINDOW(pc->scrollw)))) {
        view_height_without_bar += hscrollbar_height;
    }
    gint view_width_with_bar = view_width_without_bar - vscrollbar_width;
    gint view_height_with_bar = view_height_without_bar - hscrollbar_height;

    gdouble scale_height_without_bar = (view_height_without_bar -
            height_non_scaling) / height_scaling;
    gdouble scale_full_height_without_bar = (view_height_without_bar -
            full_height_non_scaling) / full_height_scaling;
    gdouble scale_width_without_bar = (view_width_without_bar -
            width_non_scaling)  / width_scaling;
    gdouble scale_height_with_bar = (view_height_with_bar -
            height_non_scaling) / height_scaling;
    gdouble scale_width_with_bar = (view_width_with_bar - width_non_scaling) /
        width_scaling;
    gdouble scale_both = MIN(scale_width_without_bar, scale_height_without_bar);
    gdouble scale_both_full = MIN(scale_width_without_bar,
            scale_full_height_without_bar);

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
        scale_both = MAX(scale_both_full, MIN(scale_width_with_bar,
                    scale_height_without_bar));
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

    slog(L_DEBUG, "Document size wrong for fitting, changing scale from %f "
            "to %f.\n", pc->scale, scale);

    // We do not really know where to center the scroll that might appear, 
    // passing the center of the window causes the toolbar to not be darn 
    // (don't ask me why).
    // Passing NAN as position to center the scrolling on, causes no 
    // scrolling to happen (this is checked in previewgui_goto_xy)
    // So this is basically a bugfix - but I could not see any unwanted side
    // effects up till now...
    previewgui_set_scale(pc, scale,
        NAN,
        NAN);
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
    //L_F_DEBUG;

    pc->current_page = page;
    
    update_page_input(pc);

}

static void update_page_input(GuPreviewGui* pc) {

    if (!gtk_widget_has_focus(pc->page_input)) {
        gchar* num = g_strdup_printf ("%d", pc->current_page+1);
        g_signal_handler_block(pc->page_input, pc->page_input_changed_handler);
        gtk_entry_set_text (GTK_ENTRY(pc->page_input), num);
        g_signal_handler_unblock(pc->page_input,pc->page_input_changed_handler);
        g_free (num);
    }

    update_prev_next_page(pc);

}

static void update_page_positions(GuPreviewGui* pc) {

    LayeredRectangle fov = get_fov(pc);
    int i;

    if (is_continuous(pc)) {
        gint y = get_document_margin(pc);
        
        for (i=0; i<pc->n_pages; i++) {
            page_inner(pc, i).y = y;
            page_inner(pc, i).width = get_page_width(pc, i)*pc->scale;
            page_inner(pc, i).x = MAX((fov.width - page_inner(pc, i).width)/2, 
                                       get_document_margin(pc));
            page_inner(pc, i).height = get_page_height(pc, i)*pc->scale;
            page_inner(pc, i).layer = 0;
            
            y += page_inner(pc, i).height + get_page_margin(pc);
        }
        
        y -= get_page_margin(pc);
        y += get_document_margin(pc);
        
        if (y < fov.height) {
            gint diff = (fov.height - y) / 2;
            for (i=0; i<pc->n_pages; i++) {
                page_inner(pc, i).y += diff;
            }
        }
    } else {
        
        for (i=0; i<pc->n_pages; i++) {
            page_inner(pc, i).height = get_page_height(pc, i)*pc->scale;
            page_inner(pc, i).width = get_page_width(pc, i)*pc->scale;
            page_inner(pc, i).y = MAX((fov.height - page_inner(pc, i).height)/2,
                                       get_document_margin(pc));
            page_inner(pc, i).x = MAX((fov.width - page_inner(pc, i).width)/2, 
                                       get_document_margin(pc));
            page_inner(pc, i).layer = i;
        }
        
    }
    
    for (i=0; i<pc->n_pages; i++) {
        page_outer(pc, i).x = page_inner(pc, i).x - 1;
        page_outer(pc, i).y = page_inner(pc, i).y - 1;
        page_outer(pc, i).width = page_inner(pc, i).width + PAGE_SHADOW_WIDTH;
        page_outer(pc, i).height = page_inner(pc, i).height + PAGE_SHADOW_WIDTH;
        page_outer(pc, i).layer = page_inner(pc, i).layer;
    }
}

static gboolean on_page_input_lost_focus(GtkWidget *widget, GdkEvent  *event, 
                                         gpointer   user_data) {
    update_page_input(user_data);
    return FALSE;
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
    //L_F_DEBUG;

    gdouble offset_y = MAX(get_document_margin(pc),
            (gtk_adjustment_get_page_size(pc->vadj) - pc->height_scaled)/2 );

    // TODO: This can be simplified...

    // The page margins are just for safety...
    gdouble view_start_y = gtk_adjustment_get_value(pc->vadj) -
        get_page_margin(pc);
    gdouble view_end_y   = view_start_y + gtk_adjustment_get_page_size(pc->vadj)
        + 2*get_page_margin(pc);

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
    //L_F_DEBUG;

    int i;
    for (i = 0; i < pc->n_pages; i++) {
        remove_page_rendering(pc, i);
    }
    
    if (pc->cache_size != 0) {
        slog(L_ERROR, "Cleared all page renderings, but cache not empty. "
                "Cache size is %iB.\n", pc->cache_size);
    }
    
}

static gboolean remove_page_rendering(GuPreviewGui* pc, gint page) {
    if ((pc->pages + page)->rendering == NULL) {
        return FALSE;
    }
    //L_F_DEBUG;

    cairo_surface_destroy((pc->pages + page)->rendering);
    (pc->pages + page)->rendering = NULL;
    pc->cache_size -= page_inner(pc, page).width * 
            page_inner(pc, page).height * BYTES_PER_PIXEL; 
            
    return TRUE;
}

static void update_drawarea_size(GuPreviewGui *pc) {
    //L_F_DEBUG;

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
    gtk_adjustment_set_upper(pc->hadj,
        (width==1) ? gtk_adjustment_get_page_size(pc->hadj) : width);
    gtk_adjustment_set_upper(pc->vadj,
        (height==1) ? gtk_adjustment_get_page_size(pc->vadj) : height);
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
    //L_F_DEBUG;

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
    //L_F_DEBUG;

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
    update_page_positions(pc);
}

static void update_scaled_size(GuPreviewGui* pc) {
    //L_F_DEBUG;

    if (is_continuous(pc)) {
        pc->height_scaled = pc->height_pages*pc->scale + (pc->n_pages-1) * get_page_margin(pc);
    } else {
        pc->height_scaled = get_page_height(pc, pc->current_page) * pc->scale;
    }
    
    

    pc->width_scaled = pc->width_pages*pc->scale;
}

static void previewgui_set_scale(GuPreviewGui* pc, gdouble scale, gdouble x,
    gdouble y) {

    if (pc->scale == scale) {
        return;
    }
    //L_F_DEBUG;

    gdouble old_x = (gtk_adjustment_get_value(pc->hadj) + x) /
            (pc->width_scaled + 2*get_document_margin(pc));
    gdouble old_y = (gtk_adjustment_get_value(pc->vadj) + y) /
            (pc->height_scaled + 2*get_document_margin(pc));

    // We have to do this before changing the scale, as otherwise the cache 
    // size would be calcualted wrong!
    previewgui_invalidate_renderings(pc);

    pc->scale = scale;

    update_scaled_size(pc);
    update_page_positions(pc);

    // TODO: Blocking the expose event is porbably not the best way.
    // It would be great if we could change all 3 porperties(hadj, vadj & scale)
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
        gdouble new_x = old_x*(pc->width_scaled + 2*get_document_margin(pc))-x;
        gdouble new_y = old_y*(pc->height_scaled + 2*get_document_margin(pc))-y;

        previewgui_goto_xy(pc, new_x, new_y);
    }
    g_signal_handler_unblock(pc->drawarea, pc->on_expose_handler);

    gtk_widget_queue_draw (pc->drawarea);

}

static void load_document(GuPreviewGui* pc, gboolean update) {
    //L_F_DEBUG;

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
    //L_F_DEBUG;
    previewgui_cleanup_fds (pc);

    pc->uri = g_strconcat (urifrmt, pdffile, NULL);

    pc->doc = poppler_document_new_from_file (pc->uri, NULL, NULL);
    g_return_if_fail (pc->doc != NULL);

    pc->restore_x = -1;
    pc->restore_y = -1;

    load_document(pc, FALSE);   

    // This is mainly for debugging - to make sure the boxes in the preview disappear.    
    synctex_clear_sync_nodes(pc);

    // Restore scale and fit mode
    g_signal_handler_block(pc->combo_sizes, pc->combo_sizes_changed_handler);
    if (STR_EQU (config_get_value ("zoommode"), "pagewidth")) {
        set_fit_mode(pc, FIT_WIDTH);
        gtk_combo_box_set_active(pc->combo_sizes, ZOOM_FIT_WIDTH);
    } else if (STR_EQU (config_get_value ("zoommode"), "pageheight")) {
        set_fit_mode(pc, FIT_HEIGHT);
        //gtk_combo_box_set_active(pc->combo_sizes, new_index);
    } else if (STR_EQU (config_get_value ("zoommode"), "bestfit")) {
        set_fit_mode(pc, FIT_BOTH);
        gtk_combo_box_set_active(pc->combo_sizes, ZOOM_FIT_BOTH);
    } else {
        set_fit_mode(pc, FIT_NONE);
        previewgui_set_scale(pc, list_sizes[ZOOM_100],
                NAN,    // We pass NAN as this causes no scrolling to happen
                NAN);   // This is checked in previewgui_goto_xy()
                        // This might also have caused Bug #252.
        gtk_combo_box_set_active(pc->combo_sizes, ZOOM_100);
    }
    g_signal_handler_unblock(pc->combo_sizes, pc->combo_sizes_changed_handler);

    gtk_widget_queue_draw (pc->drawarea);
    
    previewgui_goto_page (pc, 0);
}

void previewgui_refresh (GuPreviewGui* pc, GtkTextIter *sync_to,
        gchar* tex_file) {
    /* We lock the mutex to prevent previewing imcomplete PDF file, i.e
     * compiling. Also prevent PDF from changing (compiling) when previewing */
    if (!g_mutex_trylock (gummi->motion->compile_mutex)) return;

    /* This line is very important, if no pdf exist, preview will fail */
    if (!pc->uri || !utils_path_exists (pc->uri + usize)) goto unlock;

    previewgui_cleanup_fds (pc);

    pc->doc = poppler_document_new_from_file (pc->uri, NULL, NULL);

    /* release mutex and return when poppler doc is damaged or missing */
    if (pc->doc == NULL) goto unlock;

    load_document(pc, TRUE);
    update_page_positions(pc);

    if (config_get_value ("synctex") && config_get_value ("autosync") && 
            synctex_run_parser(pc, sync_to, tex_file)) {
        
        
        SyncNode *node;
        if ((node = synctex_one_node_found(pc)) == NULL) {
            // See if the nodes are so close they all fit in the window 
            // in that case we just merge them
            synctex_merge_nodes(pc);
        }
        
#if HAVE_POPPLER_PAGE_GET_SELECTED_TEXT
        if ((node = synctex_one_node_found(pc)) == NULL) {
            // Search for words in the pdf
            synctex_filter_results(pc, sync_to);
        }
        // Here we could try merging again - but only with nodes which 
        // contained the searched text
#endif
        
        // If we have only one node left/selected, scroll ot it.
        if ((node = synctex_one_node_found(pc)) != NULL) {
           synctex_scroll_to_node(pc, node);
        }
        
    } else {
        
        // This is mainly for debugging - to make sure the boxes in the preview disappear.  
        synctex_clear_sync_nodes(pc);
        
        if (pc->current_page >= pc->n_pages) {
            previewgui_goto_page (pc, pc->n_pages-1);
        }

    }

    gtk_widget_queue_draw (pc->drawarea);

unlock:
    g_mutex_unlock (gummi->motion->compile_mutex);
}

static gboolean synctex_run_parser(GuPreviewGui* pc, GtkTextIter *sync_to, gchar* tex_file) {

    if (sync_to == NULL || tex_file == NULL) {
        return FALSE;
    }

    // sync to position...
    gint line = gtk_text_iter_get_line(sync_to)+1; // SyncTeX lines are 1 based, TextBuffer lines are 0 based
    gint column = gtk_text_iter_get_line_offset(sync_to);
    slog(L_DEBUG, "Syncing to %s, line %i, column %i\n", tex_file, line, column);

    synctex_scanner_t sync_scanner = synctex_scanner_new_with_output_file(pc->uri, C_TMPDIR, 1);

    synctex_clear_sync_nodes(pc);

    if(synctex_display_query(sync_scanner, tex_file, line, column)>0) {
        synctex_node_t node;
        /*
         * SyncTeX can return several nodes. It seems best to use the last one, as
         * this one rarely is below (usually slighly above) the edited line.
         */
         
        while ((node = synctex_next_result(sync_scanner))) {
        
            SyncNode *sn = g_new0(SyncNode, 1);
        
            sn->page = synctex_node_page(node) - 1; // syncTeX counts from 1, but poppler from 0
            sn->x = synctex_node_box_visible_h(node);
            sn->y = synctex_node_box_visible_v(node);
            sn->width = synctex_node_box_visible_width(node);
            sn->height = synctex_node_box_visible_height(node);
            sn->y -= sn->height;    // We want y to be the upper value
            
            pc->sync_nodes = g_slist_append(pc->sync_nodes, sn);
            
        }
    }

    synctex_scanner_free(sync_scanner);
    
    return TRUE;    
}

#if HAVE_POPPLER_PAGE_GET_SELECTED_TEXT
static void synctex_filter_results(GuPreviewGui* pc, GtkTextIter *sync_to) {

    // First look if we even have to filter...
    if (g_slist_length(pc->sync_nodes) == 0) {
        return;
    }

    GtkTextIter wordStart = *sync_to;
    int i;
    for (i=0; i<5; i++) {
    
        gtk_text_iter_backward_word_start(&wordStart);
        
        GtkTextIter wordEnd = wordStart;
        gtk_text_iter_forward_word_end(&wordEnd);
        

        if (gtk_text_iter_compare(&wordStart, &wordEnd) >= 0) {
            break;
        }
            
        gchar *word = g_strconcat("\\b", gtk_text_iter_get_text(&wordStart, &wordEnd), "\\b", NULL);
        
        //gchar *pattern g_strconcat
        
        slog(L_DEBUG, "Searching for word \"%s\"\n", word);

        GSList *nl = pc->sync_nodes;
        
        while (nl != NULL) {
        
            SyncNode *sn =  nl->data;
            
            PopplerRectangle selection;
            selection.x1 = sn->x;               // lower left corner
            selection.y1 = sn->y + sn->height;  // lower left corner
            selection.x2 = sn->x + sn->width;   // upper right corner
            selection.y2 = sn->y;               // upper right corner
            
            PopplerPage* ppage = poppler_document_get_page(pc->doc, sn->page);
            gchar *node_text = poppler_page_get_selected_text(ppage, 
                        POPPLER_SELECTION_WORD, &selection);
            
            //slog(L_DEBUG, "Node contains text\"%s\"\n", node_text);
            
            if (g_regex_match_simple(word, node_text, 0, 0)) {
                sn->score += 1;
            }
            
            g_free(node_text);
            g_object_unref(ppage);
        
            nl = nl->next;
        }
        
        g_free(word);
    }
}
#endif


static SyncNode* synctex_one_node_found(GuPreviewGui* pc) {
    
    if (g_slist_length(pc->sync_nodes) == 1) {
        SyncNode *node = g_slist_nth_data(pc->sync_nodes, 0);
        node->score = -1;
        return node;
    }
    
    // See if we have found a single match
    GSList *nl = pc->sync_nodes;
    
    gint score_max_id = -1;
    gint score_other = 0;
    gint n = 0;
    while (nl != NULL) {
        SyncNode *sn =  nl->data;
    
        if (sn->score > score_other) {
            score_other = sn->score;
            score_max_id = n;
        } else if (sn->score == score_other) {
            // If we find a second node with the same score, we forget about
            // the first one..
            score_max_id = -1;
        }
    
        nl = nl->next;
        n++;
    }
    
    if (score_max_id >= 0) {
        SyncNode *node = g_slist_nth_data(pc->sync_nodes, score_max_id);
        node->score = -1;
        return node;
    }
    
    return NULL;
}

static void synctex_merge_nodes(GuPreviewGui* pc) {

    gint x1 = INT_MAX;   // upper left corner
    gint y1 = INT_MAX;   // upper left corner
    gint x2 = -1;   // lower right corner
    gint y2 = -1;   // lower right corner
    
    gint page = -1;
    
    GSList *nl = pc->sync_nodes;
    
    while (nl != NULL) {
    
        SyncNode *sn =  nl->data;
        
        
        slog(L_DEBUG, "Nodes (%i, %i), w=%i, h=%i, P=%i\n", sn->x, sn->y, sn->width, sn->height, sn->page);
        
        if (page == -1) {
            page = sn->page;
        } else if (page != sn->page) {
            return; // The Nodes are on different pages. We don't hande this for now...
        }
        
        x1 = MIN(x1, sn->x);
        y1 = MIN(y1, sn->y);
        x2 = MAX(x2, sn->x + sn->width);
        y2 = MAX(y2, sn->y + sn->height);
    
        nl = nl->next;
    }
    
    if ((y2-y1)*pc->scale < gtk_adjustment_get_page_size(pc->vadj)/3) {
        SyncNode *sn = g_new0(SyncNode, 1);
        sn->y = y1;
        sn->x = x1;
        
        sn->width = x2 - x1;
        sn->height = y2 - y1;
        sn->page = page;
        
        slog(L_DEBUG, "Merged nodes to (%i, %i), w=%i, h=%i, p=%i\n", sn->x, sn->y, sn->width, sn->height, sn->page);
        
        synctex_clear_sync_nodes(pc);
        pc->sync_nodes = g_slist_append(pc->sync_nodes, sn);
    }
    
}

static void synctex_clear_sync_nodes(GuPreviewGui* pc) {
    GSList *el = pc->sync_nodes;
    while (el != NULL) {
        SyncNode *node = el->data;
        g_free(node);
        node = NULL;
        
        el = el->next;
    }
    
    g_slist_free (pc->sync_nodes);
    pc->sync_nodes = NULL;
}

static void synctex_scroll_to_node(GuPreviewGui* pc, SyncNode* node) {

    gint adjpage_width = gtk_adjustment_get_page_size(pc->hadj);
    gint adjpage_height = gtk_adjustment_get_page_size(pc->vadj);

    gdouble node_x = MAX(get_document_margin(pc),
                          (adjpage_width - pc->width_scaled) / 2);
    gdouble node_y;
    
    if (is_continuous(pc)) {
        node_y = MAX(get_document_margin(pc),
                               (adjpage_height - pc->height_scaled) / 2);

        int i;
        for (i=0; i < node->page; i++) {
            node_y += get_page_height(pc, i)*pc->scale + get_page_margin(pc);
        }
    } else {
        gdouble height = get_page_height(pc, pc->current_page) * pc->scale;
        node_y = MAX(get_document_margin(pc), (adjpage_height-height)/2);
    }
    
    node_y += node->y * pc->scale;
    node_x += node->x * pc->scale;
    gdouble node_height = node->height * pc->scale;
    gdouble node_width = node->width * pc->scale;

    gdouble view_x = gtk_adjustment_get_value(pc->hadj);
    gdouble view_width = adjpage_width;
    gdouble view_y = gtk_adjustment_get_value(pc->vadj);
    gdouble view_height = adjpage_height;
    
    slog(L_DEBUG, "node: (%f, %f), w=%f, h=%f\n", node_x, node_y, node_width,
            node_height);
    slog(L_DEBUG, "view: (%f, %f), w=%f, h=%f\n", view_x, view_y,
        view_width, view_height);

    gdouble to_y;
    gdouble to_x;
    // Positioning algorithm:
    // The x and y coordinates are treated separately.  For each,
    //  - If the node is already within the view, do not change the view.
    //  - Else, if the node can fit in the view, center it.
    //  - Else, align the view to the top/left of the view.
    // The functions used to change the view do bounds checking, so we
    // don't do that here.

    if (node_y > view_y && node_y + node_height < view_y + view_height) {
        to_y = view_y;
    } else if (node_height < view_height) {
        to_y = node_y + (node_height - view_height)/2;
    } else {
        to_y = node_y;
    }
        
    if (node_x > view_x && node_x + node_width < view_x + view_width) {
        to_x = view_x;
    } else if (node_width < view_width) {
        to_x = node_x + (node_width - view_width)/2;
    } else {
        to_x = node_x;
    }
    
    if (!is_continuous(pc) && pc->current_page != node->page) {
    
        previewgui_goto_page (pc, node->page);
        previewgui_goto_xy(pc, to_x, to_y);
    
    } else {
        if (STR_EQU (config_get_value ("animated_scroll"), "always") ||
            STR_EQU (config_get_value ("animated_scroll"), "autosync")) {
            previewgui_scroll_to_xy(pc, to_x, to_y);
        } else {
            previewgui_goto_xy(pc, to_x, to_y);
        }
    }

}

void previewgui_goto_page (GuPreviewGui* pc, int page) {
    //L_F_DEBUG;
    page = MAX(page, 0);
    page = MIN(page, pc->n_pages-1);

    previewgui_set_current_page(pc, page);

    gint i;
    gdouble y = 0;

    if (!is_continuous(pc)) {
        update_scaled_size(pc);
        update_drawarea_size(pc);
    } else {
        for (i=0; i < page; i++) {
            y += get_page_height(pc, i)*pc->scale + get_page_margin(pc);
        }
    }

    //previewgui_goto_xy(pc, page_offset_x(pc, page, 0),
    //                       page_offset_y(pc, page, y));
    // We do not want to scroll horizontally.
    previewgui_goto_xy(pc, gtk_adjustment_get_value(pc->hadj),
                           page_offset_y(pc, page, y));

    if (!is_continuous(pc)) {
        gtk_widget_queue_draw (pc->drawarea);
    }
}

void previewgui_scroll_to_page (GuPreviewGui* pc, int page) {
    //L_F_DEBUG;

    if (!is_continuous(pc)) {
        // We do not scroll in single page mode...
        previewgui_goto_page(pc, page);
        return;
    }

    page = MAX(page, 0);
    page = MIN(page, pc->n_pages-1);

    previewgui_set_current_page(pc, page);

    gint i;
    gdouble y = 0;
    for (i=0; i < page; i++) {
        y += get_page_height(pc, i)*pc->scale + get_page_margin(pc);
    }

    //previewgui_scroll_to_xy(pc, page_offset_x(pc, page, 0),
    //                       page_offset_y(pc, page, y));
    // We do not want to scroll horizontally in single paged mode...
    previewgui_scroll_to_xy(pc, gtk_adjustment_get_value(pc->hadj),
                           page_offset_y(pc, page, y));
}

void previewgui_goto_xy (GuPreviewGui* pc, gdouble x, gdouble y) {

    if (isnan(x) || isnan(y)) {
        return;
    }
    //L_F_DEBUG;

    x = CLAMP(x, 0, gtk_adjustment_get_upper(pc->hadj) - 
                    gtk_adjustment_get_page_size(pc->hadj));
    y = CLAMP(y, 0, gtk_adjustment_get_upper(pc->vadj) - 
                    gtk_adjustment_get_page_size(pc->vadj));
        
    // Minimize the number of calls to on_adjustment_changed
    block_handlers_current_page(pc);
    gtk_adjustment_set_value(pc->hadj, x);
    gtk_adjustment_set_value(pc->vadj, y);
    gtk_adjustment_value_changed(pc->hadj);
    unblock_handlers_current_page(pc);
    gtk_adjustment_value_changed(pc->vadj);

}

void previewgui_scroll_to_xy (GuPreviewGui* pc, gdouble x, gdouble y) {

    if (isnan(x) || isnan(y)) {
        return;
    }
    //L_F_DEBUG;

    x = CLAMP(x, 0, gtk_adjustment_get_upper(pc->hadj) -
               gtk_adjustment_get_page_size(pc->hadj));
    y = CLAMP(y, 0, gtk_adjustment_get_upper(pc->vadj) -
               gtk_adjustment_get_page_size(pc->vadj));

    pc->ascroll_steps_left = ASCROLL_STEPS;

    pc->ascroll_end_x = x;
    pc->ascroll_end_y = y;

    pc->ascroll_dist_x = gtk_adjustment_get_value(pc->hadj) - x;
    pc->ascroll_dist_y = gtk_adjustment_get_value(pc->vadj) - y;

    g_timeout_add (1000./25., previewgui_animated_scroll_step, pc);

}

void previewgui_save_position (GuPreviewGui* pc) {
    //L_F_DEBUG;
    /* update last scroll position to restore it after error mode */
    pc->restore_y = gtk_adjustment_get_value(pc->vadj);
    pc->restore_x = gtk_adjustment_get_value(pc->hadj);
    block_handlers_current_page(pc);

    slog(L_DEBUG, "Position SAVED\n\n\n");
}

void previewgui_restore_position (GuPreviewGui* pc) {
    //L_F_DEBUG;
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
        pc->cache_size += page_inner(pc, page).width * 
                page_inner(pc, page).height * BYTES_PER_PIXEL;
    
        // Trigger the garbage collector to be run - it will exit if nothing is TBD.    
        g_idle_add( (GSourceFunc) run_garbage_collector, pc);
    }

    return cairo_surface_reference(p->rendering);
}

void previewgui_reset (GuPreviewGui* pc) {
    //L_F_DEBUG;
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
    //L_F_DEBUG;

    if (pc->doc) {
        g_object_unref (pc->doc);
        pc->doc = NULL;
    }
}

void previewgui_start_preview (GuPreviewGui* pc) {
    if (STR_EQU (config_get_value ("compile_scheme"), "on_idle")) {
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
    //L_F_DEBUG;

    gint newpage = atoi (gtk_entry_get_text (entry));
    newpage -= 1;
    newpage = MAX(newpage, 0);
    newpage = MIN(newpage, gui->previewgui->n_pages);
    
    if (STR_EQU (config_get_value ("animated_scroll"), "always")) {
        previewgui_scroll_to_page (gui->previewgui, newpage);
    } else {
        previewgui_goto_page (gui->previewgui, newpage);
    }

}

G_MODULE_EXPORT
void on_next_page_clicked (GtkWidget* widget, void* user) {
    //L_F_DEBUG;
    GuPreviewGui *pc = gui->previewgui;

    if (STR_EQU (config_get_value ("animated_scroll"), "always")) {
        previewgui_scroll_to_page (pc, pc->next_page);
    } else {
        previewgui_goto_page (pc, pc->next_page);
    }
}

G_MODULE_EXPORT
void on_prev_page_clicked (GtkWidget* widget, void* user) {
    //L_F_DEBUG;
    GuPreviewGui *pc = gui->previewgui;

    if (STR_EQU (config_get_value ("animated_scroll"), "always")) {
        previewgui_scroll_to_page (pc, pc->prev_page);
    } else {
        previewgui_goto_page (pc, pc->prev_page);
    }
}

G_MODULE_EXPORT
void on_combo_sizes_changed (GtkWidget* widget, void* user) {
    //L_F_DEBUG;
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

static void paint_page (cairo_t *cr, GuPreviewGui* pc, gint page, gint x, gint y) {
    if (page < 0 || page >= pc->n_pages) {
        return;
    }

    //slog (L_DEBUG, "printing page %i at (%i, %i)\n", page, x, y);

    gdouble page_width = get_page_width(pc, page) * pc->scale;
    gdouble page_height = get_page_height(pc, page) * pc->scale;

    // Paint shadow
    cairo_set_source_rgb (cr, 0.302, 0.302, 0.302);
    cairo_rectangle (cr, x + page_width , y + PAGE_SHADOW_OFFSET ,
                     PAGE_SHADOW_WIDTH, page_height);
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


    GSList *nl = pc->sync_nodes;
    while (nl != NULL && in_debug_mode()) {
    
        SyncNode *sn =  nl->data;
    
        if (sn->page == page) {
            gint mark_x = sn->x * pc->scale;
            gint mark_y = sn->y * pc->scale;
            gint mark_width = sn->width * pc->scale;
            gint mark_height = sn->height * pc->scale;
        
            cairo_set_line_width (cr, 1);
            if (sn->score < 0) {
                cairo_set_source_rgb (cr, 1, 0, 0); // Mark selected node red
            } else if (sn->score > 0) {
                cairo_set_source_rgb (cr, 0, 1, 0); // Mark nodes with matches green
            } else {
                cairo_set_source_rgb (cr, 0, 0, 1); // Mark other nodes blue
            }
            cairo_rectangle (cr, x+mark_x-1, y+mark_y-1, mark_width+2, mark_height+2);
            cairo_stroke (cr);
        }
        
        nl = nl->next;
    }
    
    cairo_surface_destroy(rendering);
}

static inline LayeredRectangle get_fov(GuPreviewGui* pc) {
    LayeredRectangle fov;
    fov.x = gtk_adjustment_get_value(pc->hadj);
    fov.y = gtk_adjustment_get_value(pc->vadj);
    fov.width = gtk_adjustment_get_page_size(pc->hadj); // TODO: Validate this is alsways correct
    fov.height = gtk_adjustment_get_page_size(pc->vadj);// TODO: Validate this is alsways correct
    if (is_continuous(pc)) {
        fov.layer = 0;
    } else {
        fov.layer = pc->current_page;
    }
    
    return fov;
}

/**
 *  Tests for the intersection of both rectangles src1 and src2.
 *  If dest is set and there is a intersection, it will be the intersecting, 
 *  rectangle. If dest is set but src1 and src2 do not intersect, dest's width
 *  and height will be set to 0. All other values will be undefined. Dest may be
 *  the same as src1 or src2.
 *
 *  Set dest to NULL, if you are only interested in the boolean result.
 */
static gboolean layered_rectangle_intersect(const LayeredRectangle *src1,
                                            const LayeredRectangle *src2,
                                            LayeredRectangle *dest) {

    if (src1 == NULL || src2 == NULL) {
        if (dest) {
            dest->width = 0;
            dest->height = 0;
        }
        return FALSE;
    }
                                  
    if (src1->layer == src2->layer) {
   
        gint dest_x = MAX (src1->x, src2->x);
        gint dest_y = MAX (src1->y, src2->y);
        gint dest_x2 = MIN (src1->x + src1->width, src2->x + src2->width);
        gint dest_y2 = MIN (src1->y + src1->height, src2->y + src2->height);
        
        if (dest_x2 > dest_x && dest_y2 > dest_y) {
            if (dest) {
                dest->x = dest_x;
                dest->y = dest_y;
                dest->width = dest_x2 - dest_x;
                dest->height = dest_y2 - dest_y;
                dest->layer = src1->layer;
            }
            return TRUE;
        }
    }
    
    if (dest) {
        dest->width = 0;
        dest->height = 0;
    }
    return FALSE;
}

/**
 *  Calculates the union of the rectangles src1 and src2.
 *
 *  Returns TRUE if both src rectangles are defined and on the same layer.
 *  If dest is not NULL, it will be the union of both rectangles, if however the
 *  src rectangles are on different layers, dest's layer will be undefined.
 */
/*
static gboolean layered_rectangle_union(const LayeredRectangle *src1,
                                        const LayeredRectangle *src2,
                                        LayeredRectangle *dest) {

    if (src1 == NULL || src2 == NULL) {
        if (dest) {
            dest->width = 0;
            dest->height = 0;
        }
        return FALSE;
    }
    
    if (dest) {
    
        gint dest_x = MIN (src1->x, src2->x);
        gint dest_y = MIN (src1->y, src2->y);
        gint dest_x2 = MAX (src1->x + src1->width, src2->x + src2->width);
        gint dest_y2 = MAX (src1->y + src1->height, src2->y + src2->height);
    
        dest->x = dest_x;
        dest->y = dest_y;
        dest->width = dest_x2 - dest_x;
        dest->height = dest_y2 - dest_y;
        if (src1->layer == src2->layer) {
            dest->layer = src1->layer;
        }
    }
    
    if (src1->layer != src2->layer) {
        return FALSE;
    }
    return TRUE;
}*/

gboolean run_garbage_collector(GuPreviewGui* pc) {
    
    gint max_cache_size = atoi (config_get_value ("cache_size")) * 1024 * 1024;
    
    if (pc->cache_size < max_cache_size) {
        return FALSE;
    }
    
    LayeredRectangle fov = get_fov(pc);
    
    gint first = -1;
    gint last = -1;
    
    gint i;
    for (i=0; i < pc->n_pages; i++) {
        if (layered_rectangle_intersect(&fov, &(page_inner(pc, i)), NULL)) {
            if (first == -1) {
                first = i;
            }
            last = i;
        }
    }
    
    if (first == -1) {
        slog(L_ERROR, "No pages are shown. Clearing whole cache.\n");
        previewgui_invalidate_renderings(pc);
    }
    
    gint n=0;
    gint dist = MAX(first, pc->n_pages - 1 - last);
    for (; dist > 0; dist--) {
        gint up = first - dist;
        if (up >= 0 && up < pc->n_pages) {
            if (!layered_rectangle_intersect(&fov, &(page_inner(pc, up)), NULL)) {
                if (remove_page_rendering(pc, up)) {
                    n += 1;
                }
            }
        }
        if (pc->cache_size < max_cache_size / 2) {
            break;
        }
        
        gint down = last + dist;
        if (down < pc->n_pages && down >= 0) {
            if (!layered_rectangle_intersect(&fov, &(page_inner(pc, down)), NULL)) {
                if (remove_page_rendering(pc, down)) {
                    n += 1;
                }
            }
        }
        if (pc->cache_size < max_cache_size / 2) {
            break;
        }
    }
    
    if (n == 0) {
        slog(L_DEBUG, "Could not delete any pages from cache. All pages are "
                "currently visible.\n");
    } else {
        slog(L_DEBUG, "Deleted %i pages from cache.\n", n);
    }
        
    return FALSE;   // We only want this to run once - so always return false!
}

G_MODULE_EXPORT
gboolean on_expose (GtkWidget* w, GdkEventExpose* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);

//    slog(L_INFO, "paint document with scale %f, region (%i, %i), w=%i,
//    h=%i\n", e->area.x, e->area.y, e->area.width, e->area.height);

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) {

        return FALSE;
    }

    gdouble page_width = gtk_adjustment_get_page_size(pc->hadj);
    gdouble page_height = gtk_adjustment_get_page_size(pc->vadj);

    gdouble offset_x = MAX(get_document_margin(pc),
                          (page_width - pc->width_scaled) / 2);

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(w));

    if (is_continuous(pc)) {

        gdouble offset_y = MAX(get_document_margin(pc),
                               (page_height - pc->height_scaled) / 2);

        // The page margins are just for safety...
        gdouble view_start_y = gtk_adjustment_get_value(pc->vadj) -
                               get_page_margin(pc);
        gdouble view_end_y = view_start_y + page_height + 2*get_page_margin(pc);

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
        gdouble offset_y = MAX(get_document_margin(pc), (page_height-height)/2);

        paint_page(cr, pc, pc->current_page,
            page_offset_x(pc, pc->current_page, offset_x),
            page_offset_y(pc, pc->current_page, offset_y));
    }

    cairo_destroy (cr);

    return TRUE;
}

G_MODULE_EXPORT
void on_adj_changed(GtkAdjustment *adjustment, gpointer user) {
    //L_F_DEBUG;
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);

    // Abort any animated scrolls that might be running...
    pc->ascroll_steps_left = 0;

    update_current_page(pc);
}

G_MODULE_EXPORT
gboolean on_scroll (GtkWidget* w, GdkEventScroll* e, void* user) {
    //L_F_DEBUG;
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;

    if (GDK_CONTROL_MASK & e->state) {

        gdouble old_scale = pc->scale;
        gdouble new_scale = -1;
        gint    new_index = -1;
        int i;

        // we only go through the percentage entrys - the fit entrys are not
        // always uo to date...
        for (i=0; i<N_ZOOM_SIZES; i++) {
            if (i == ZOOM_FIT_WIDTH || i == ZOOM_FIT_BOTH) {
                continue;
            }
            if (list_sizes[i] > old_scale && e->direction == GDK_SCROLL_UP) {
                if (new_index == -1 || list_sizes[i] < new_scale) {
                    new_scale = list_sizes[i];
                    new_index = i;
                }
            } else if (list_sizes[i] < old_scale &&
                       e->direction == GDK_SCROLL_DOWN) {
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
            g_signal_handler_block(pc->combo_sizes,
                                   pc->combo_sizes_changed_handler);
            gtk_combo_box_set_active(pc->combo_sizes, new_index);
            g_signal_handler_unblock(pc->combo_sizes,
                                     pc->combo_sizes_changed_handler);

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
            switch (e->direction) {
                case GDK_SCROLL_UP:

                    if (pc->prev_page != -1) {
                        previewgui_goto_page (pc, pc->prev_page);
                    }

                    break;
                case GDK_SCROLL_DOWN:

                    if (pc->next_page != -1) {
                        previewgui_goto_page (pc, pc->next_page);
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

static void draw2page(GuPreviewGui* pc, gint dx, gint dy, gint *pp, gint *px, gint *py) {

    *px = dx;
    *py = dy;
    *pp = 0;

    gint adjpage_width = gtk_adjustment_get_page_size(pc->hadj);
    gint adjpage_height = gtk_adjustment_get_page_size(pc->vadj);

    *px -= MAX(get_document_margin(pc),
                          (adjpage_width - pc->width_scaled) / 2);
    
    if (is_continuous(pc)) {
        *py -= MAX(get_document_margin(pc),
                               (adjpage_height - pc->height_scaled) / 2);

        int i;
        for (i=0; i < pc->n_pages-1; i++) {
            gint pheight = get_page_height(pc, i)*pc->scale + get_page_margin(pc);
            if (*py > pheight) {
                *py -= pheight;
                *pp += 1;
            }
        }
    } else {
        gdouble height = get_page_height(pc, pc->current_page) * pc->scale;
        *py -= MAX(get_document_margin(pc), (adjpage_height-height)/2);
        *pp += pc->current_page;
    }
    
    //TODO Check if we still are inside a page...
}

G_MODULE_EXPORT
gboolean on_button_pressed(GtkWidget* w, GdkEventButton* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;

    // Check where the user clicked
    gint page;
    gint x;
    gint y;
    draw2page(pc, e->x, e->y, &page, &x, &y);
    
    if (e->state & GDK_CONTROL_MASK) {
    
        
        slog(L_DEBUG, "Ctrl-click to %i, %i\n", x, y);
    
        synctex_scanner_t sync_scanner = synctex_scanner_new_with_output_file(pc->uri, C_TMPDIR, 1);
        
        if(synctex_edit_query(sync_scanner, page+1, x/pc->scale, y/pc->scale)>0) {
            synctex_node_t node;
            /*
             * SyncTeX can return several nodes. It seems best to use the last one, as
             * this one rarely is below (usually slighly above) the edited line.
             */
             
            if ((node = synctex_next_result(sync_scanner))) {
            
                const gchar *file = synctex_scanner_get_name(sync_scanner, synctex_node_tag(node));
                gint line = synctex_node_line(node);
            
                slog(L_DEBUG, "File \"%s\", Line %i\n", file, line);
                
                // FIXME: Go to the editor containing the file "file"!
                editor_scroll_to_line(gummi_get_active_editor(), line-1);
                
            }
        }

        synctex_scanner_free(sync_scanner);
    
    }

    pc->prev_x = e->x;
    pc->prev_y = e->y;
    return FALSE;
}

G_MODULE_EXPORT
gboolean on_motion (GtkWidget* w, GdkEventMotion* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;

    gdouble new_x = gtk_adjustment_get_value (pc->hadj) - (e->x - pc->prev_x);
    gdouble new_y = gtk_adjustment_get_value (pc->vadj) - (e->y - pc->prev_y);

    previewgui_goto_xy(pc, new_x, new_y);

    return TRUE;
}

G_MODULE_EXPORT
gboolean on_resize (GtkWidget* w, GdkRectangle* r, void* user) {
    //L_F_DEBUG;
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;

    LayeredRectangle fov = get_fov(pc);
    gdouble x_rel = (gdouble) (fov.x + fov.width/2) / pc->width_scaled;
    gdouble y_rel = (gdouble) (fov.y + fov.height/2) / pc->height_scaled;

    update_fit_scale(pc);
    update_page_positions(pc);
    
    fov = get_fov(pc);
    previewgui_goto_xy (pc, x_rel*pc->width_scaled - fov.width/2, 
                            y_rel*pc->height_scaled - fov.height/2);

    return FALSE;
}
