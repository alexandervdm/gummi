/**
 * @file    gui-preview.c
 * @brief  
 *
 * Copyright (C) 2010 Gummi-Dev Team <alexvandermey@gmail.com>
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
#include "environment.h"
#include "gui/gui-main.h"
#include "motion.h"
#include "porting.h"
#include "utils.h"


/* set up uri using appropriate formatting for OS 
   http://en.wikipedia.org/wiki/File_URI_scheme#Linux */
#ifdef WIN32
    const gchar *urifrmt = "file:///";
    gint usize = 8;
#else
    const gchar *urifrmt = "file://";
    gint usize = 7;
#endif



static gfloat list_sizes[] = {-1, -1, 0.50, 0.70, 0.85, 1.0, 1.25, 1.5, 2.0,
                              3.0, 4.0};

extern Gummi* gummi;
extern GummiGui* gui;

GuPreviewGui* previewgui_init (GtkBuilder * builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuPreviewGui* p = g_new0 (GuPreviewGui, 1);
    GdkColor bg = {0, 0xed00, 0xec00, 0xeb00};
    p->previewgui_viewport =
        GTK_VIEWPORT (gtk_builder_get_object (builder, "previewgui_view"));
    p->statuslight =
        GTK_WIDGET (gtk_builder_get_object (builder, "tool_statuslight"));
    p->drawarea =
        GTK_WIDGET (gtk_builder_get_object (builder, "previewgui_draw"));
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
    p->page = NULL;

    if (g_strcmp0(config_get_value("zoommode"), "bestfit") == 0) {
        p->page_zoommode = 0;
        gtk_combo_box_set_active(p->combo_sizes, 0);
    }
    else {
        p->page_zoommode = 1;
    }
    
    p->update_timer = 0;
    p->preview_on_idle = FALSE;
    p->page_total = 0;
    p->page_current = 1;
    p->hadj = gtk_scrolled_window_get_hadjustment
                    (GTK_SCROLLED_WINDOW (p->scrollw));
    p->vadj = gtk_scrolled_window_get_vadjustment
                    (GTK_SCROLLED_WINDOW (p->scrollw));

    gtk_widget_modify_bg (p->drawarea, GTK_STATE_NORMAL, &bg); 

    /* Install event handlers */
    gtk_widget_add_events (p->drawarea, GDK_SCROLL_MASK
                                      | GDK_BUTTON_PRESS_MASK
                                      | GDK_BUTTON_MOTION_MASK);

    g_signal_connect (p->scrollw, "size-allocate", G_CALLBACK (on_resize), p);
    g_signal_connect (p->drawarea, "scroll-event", G_CALLBACK (on_scroll), p);
    g_signal_connect (p->drawarea, "expose-event", G_CALLBACK (on_expose), p);
    g_signal_connect (p->drawarea, "button-press-event",
                      G_CALLBACK (on_key_press), p);
    g_signal_connect (p->drawarea, "motion-notify-event",
                      G_CALLBACK (on_motion), p);
    
    /* The error panel is now imported from Glade. The following
     * functions re-parent the panel widgets for use in Gummi */
    GtkWidget *holder = 
        GTK_WIDGET(gtk_builder_get_object (builder, "errorwindow"));
    p->errorpanel = 
        GTK_WIDGET(gtk_builder_get_object (builder, "errorpanel"));

    gtk_container_remove(GTK_CONTAINER(holder), p->errorpanel);
    g_object_unref(holder);
    
    slog (L_INFO, "using libpoppler %s ...\n", poppler_get_version ());
    return p;
}

void previewgui_update_statuslight (const gchar* type) {
    gdk_threads_enter ();
    gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON(gui->previewgui->statuslight),
           type);
    gdk_threads_leave ();
}

void previewgui_set_pdffile (GuPreviewGui* pc, const gchar *pdffile) {
    L_F_DEBUG;
    previewgui_cleanup_fds (pc);

    pc->uri = g_strconcat (urifrmt, pdffile, NULL);

    pc->doc = poppler_document_new_from_file (pc->uri, NULL, NULL);
    g_return_if_fail (pc->doc != NULL);

    pc->page_total = poppler_document_get_n_pages (pc->doc);
    pc->page_current = 0;
    
    pc->page = poppler_document_get_page (pc->doc, pc->page_current);
    g_return_if_fail (pc->page != NULL);

    poppler_page_get_size (pc->page, &pc->page_width, &pc->page_height);
}

void previewgui_refresh (GuPreviewGui* pc) {
    L_F_DEBUG;

    /* We lock the mutex to prevent previewing imcomplete PDF file, i.e
     * compiling. Also prevent PDF from changing (compiling) when previewing */
    if (!g_mutex_trylock (gummi->motion->compile_mutex)) return;

    /* This is line is very important, if no pdf exist, preview will fail */
    if (!pc->uri || !utils_path_exists (pc->uri + usize)) goto unlock;

    previewgui_cleanup_fds (pc);

    pc->doc = poppler_document_new_from_file (pc->uri, NULL, NULL);

    /* release mutex and return when poppler doc is damaged or missing */
    if (pc->doc == NULL) goto unlock;

    pc->page_total = poppler_document_get_n_pages (pc->doc);
    previewgui_set_pagedata (pc);

    pc->page = poppler_document_get_page (pc->doc, pc->page_current);
    poppler_page_get_size (pc->page, &pc->page_width, &pc->page_height);  

    list_sizes[0] = pc->scrollw->allocation.height / pc->page_height;
    list_sizes[1] = pc->scrollw->allocation.width / pc->page_width;

    previewgui_drawarea_resize (pc);
    gtk_widget_queue_draw (pc->drawarea);

unlock:
    g_mutex_unlock (gummi->motion->compile_mutex);
}

void previewgui_set_pagedata (GuPreviewGui* pc) {
    L_F_DEBUG;
    gchar* current = NULL;
    gchar* total = NULL;

    if ( (pc->page_total - 1) > pc->page_current) {
        gtk_widget_set_sensitive (GTK_WIDGET (pc->page_next), TRUE);
    }
    else if (pc->page_current >= pc->page_total) {
        pc->page_current = pc->page_current;
        gtk_widget_set_sensitive (pc->page_prev, (pc->page_current > 0));
        gtk_widget_set_sensitive (pc->page_next,
                (pc->page_current < (pc->page_total -1)));
    }
    current = g_strdup_printf ("%d", (pc->page_current+1));
    total = g_strdup_printf (_("of %d"), pc->page_total);

    gtk_entry_set_text (GTK_ENTRY (pc->page_input), current);
    gtk_label_set_text (GTK_LABEL (pc->page_label), total);

    g_free (current);
    g_free (total);
}

void previewgui_goto_page (GuPreviewGui* pc, int page_number) {
    L_F_DEBUG;
    if (page_number < 0 || page_number >= pc->page_total) {
        slog (L_ERROR, "page_number is a negative number!\n");
        return;
    }

    pc->page_current = page_number;
    gtk_widget_set_sensitive (pc->page_prev, (page_number > 0));
    gtk_widget_set_sensitive(pc->page_next, (page_number <(pc->page_total -1)));
    previewgui_refresh (pc);
}

void previewgui_start_error_mode (GuPreviewGui* pc) {
    
    /* update last scroll position to restore it after error mode */
    pc->prev_y = gtk_adjustment_get_value(pc->vadj);
    
    if (pc->errormode) return;
    pc->errormode = TRUE;
    gtk_container_remove (GTK_CONTAINER (pc->previewgui_viewport),
            GTK_WIDGET (pc->drawarea));
    gtk_container_add (GTK_CONTAINER (pc->previewgui_viewport),
            GTK_WIDGET (pc->errorpanel));
    gtk_widget_show_all (GTK_WIDGET (pc->previewgui_viewport));
}

void previewgui_stop_error_mode (GuPreviewGui* pc) {
    
    /* restore scroll window position to value before error mode */
    /* TODO: might want to merge this with synctex funcs in future */
    gtk_adjustment_set_value(pc->vadj, pc->prev_y);
    
    if (!pc->errormode) return;
    pc->errormode = FALSE;
    g_object_ref (pc->errorpanel);
    gtk_container_remove (GTK_CONTAINER (pc->previewgui_viewport),
            GTK_WIDGET (pc->errorpanel));
    gtk_container_add (GTK_CONTAINER (pc->previewgui_viewport),
            GTK_WIDGET (pc->drawarea));
}

void previewgui_drawarea_resize (GuPreviewGui* pc) {
    gint width = 0, height = 0;
    cairo_t* cr = NULL;

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return;

    pc->page_scale = list_sizes[pc->page_zoommode];
    height = pc->page_height * pc->page_scale;
    width = pc->page_width * pc->page_scale;
    gtk_widget_set_size_request (pc->drawarea,
                     width - (pc->page_zoommode == 1) * 20,
                     height - (pc->page_zoommode == 1) * 20 * height / width);

    if (pc->surface)
        cairo_surface_destroy (pc->surface);

    pc->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                              width, height);
    cr = cairo_create (pc->surface);

    cairo_scale (cr, pc->page_scale, pc->page_scale);
    poppler_page_render (pc->page, cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_DEST_OVER);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_destroy (cr);
}

void previewgui_reset (GuPreviewGui* pc) {
    L_F_DEBUG;
    /* reset uri */
    g_free (pc->uri);
    pc->uri = NULL;
    pc->page_current = 0;

    gummi->latex->modified_since_compile = TRUE;
    previewgui_stop_preview (pc);
    motion_do_compile (gummi->motion);

    if (config_get_value ("compile_status"))
        previewgui_start_preview (pc);
}

void previewgui_cleanup_fds (GuPreviewGui* pc) {
    if (pc->page) {
        g_object_unref (pc->page);
        pc->page = NULL;
    }
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
void previewgui_page_input_changed (GtkEntry* entry, void* user) {
    gint newpage = atoi (gtk_entry_get_text (entry));

    if (0 == newpage)
        return;
    else if (newpage >= 1 &&  newpage <= gui->previewgui->page_total) {
        previewgui_goto_page (gui->previewgui, newpage -1);
    } else {
        newpage = CLAMP (newpage, 1, gui->previewgui->page_total);
        gchar* num = g_strdup_printf ("%d", newpage);
        gtk_entry_set_text (entry, num);
        g_free (num);
        previewgui_goto_page (gui->previewgui, newpage -1);
    }
}

G_MODULE_EXPORT
void previewgui_next_page (GtkWidget* widget, void* user) {
    previewgui_goto_page (gui->previewgui, gui->previewgui->page_current + 1);
}

G_MODULE_EXPORT
void previewgui_prev_page (GtkWidget* widget, void* user) {
    previewgui_goto_page (gui->previewgui, gui->previewgui->page_current - 1);
}

G_MODULE_EXPORT
void previewgui_zoom_change (GtkWidget* widget, void* user) {
    gint index = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    if (index < 0)
        slog (L_ERROR, "preview zoom level is < 0.\n");
    gui->previewgui->page_scale = list_sizes[index];
    gui->previewgui->page_zoommode = index;
    previewgui_drawarea_resize (gui->previewgui);
    gtk_widget_queue_draw (gui->previewgui->drawarea);
    
    if (index == 0) config_set_value("zoommode", "bestfit");
    if (index == 1) config_set_value("zoommode", "pagewidth");
}

gboolean on_expose (GtkWidget* w, GdkEventExpose* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    static gdouble prev_width = 0;
    gint width = 0, height = 0, area_width = 0, area_height = 0, x = 0, y = 0;
    cairo_t *cr = NULL;

    if (!pc->uri || !utils_path_exists (pc->uri + usize)) return FALSE;

    if (prev_width != pc->scrollw->allocation.width)
        previewgui_drawarea_resize (gui->previewgui);
    prev_width = pc->scrollw->allocation.width;

    width = pc->page_width * pc->page_scale;
    height = pc->page_height * pc->page_scale;
    area_width = pc->scrollw->allocation.width;
    area_height = pc->scrollw->allocation.height;

    if (area_width > width)
        x = (area_width - width) / 2;
    if (area_height > height)
        y = (area_height - height) / 2;

    cr = gdk_cairo_create (gtk_widget_get_window (w));

    cairo_set_source_rgb (cr, 0.808, 0.808, 0.808);
    cairo_rectangle (cr, 0, 0, MAX(area_width,width), MAX(area_height,height));
    cairo_fill (cr);

    cairo_set_line_width (cr, 0.5);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_rectangle (cr, x - 1, y - 1, width + 1, height + 1);
    cairo_stroke (cr);

    cairo_set_source_rgb (cr, 0.302, 0.302, 0.302);
    cairo_rectangle (cr, x, y, width + 3, height + 3);
    cairo_fill (cr);

    cairo_set_source_surface (cr, pc->surface, x, y);
    cairo_paint (cr);

    cairo_destroy (cr);
    return TRUE;
}

gboolean on_scroll (GtkWidget* w, GdkEventScroll* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    gint i = 0, index = 0, new_index = 0, fit_width = 0;
    gboolean move_to_center = FALSE;
    float prev_scale = 0, margin = 0, margin_x = 0, margin_y = 0,
          upper_x = 0, upper_y = 0;

    if (GDK_CONTROL_MASK & e->state) {
        index = gtk_combo_box_get_active (pc->combo_sizes);
        prev_scale = list_sizes[index];

        for (i = 2; i < SIZE_COUNT; ++i)
            if (list_sizes[index] < list_sizes[i]) {
                fit_width = i;
                break;
            }

        if (index < 2) {
            new_index = fit_width;
            move_to_center = TRUE;
        } else {
            new_index = index + (e->direction == GDK_SCROLL_UP)
                              - (e->direction == GDK_SCROLL_DOWN);
            move_to_center = index < fit_width && new_index >= fit_width;
        }

        new_index = CLAMP (new_index - 2, 0, 8) + 2;
        gtk_combo_box_set_active(pc->combo_sizes, new_index);
        pc->page_scale = list_sizes[new_index];
        pc->page_zoommode = new_index;

        margin = 1.0 / prev_scale * list_sizes[new_index];
        upper_x = gtk_adjustment_get_upper (pc->hadj);
        upper_y = gtk_adjustment_get_upper (pc->vadj);
        margin_x = gtk_adjustment_get_value (pc->hadj)
                   / (upper_x - pc->scrollw->allocation.width);
        margin_y = gtk_adjustment_get_value (pc->vadj)
                   / (upper_y - pc->scrollw->allocation.height);

        upper_x *= margin;
        upper_y *= margin;

        if (index != new_index) {
            if (move_to_center)
                gtk_adjustment_set_value (pc->hadj,
                    (upper_x - pc->scrollw->allocation.width) / 2 + 5);
            else
                gtk_adjustment_set_value (pc->hadj, margin_x * (upper_x
                                          - pc->scrollw->allocation.width));

            gtk_adjustment_set_value (pc->vadj, margin_y * (upper_y
                                      - pc->scrollw->allocation.height));
            gtk_adjustment_value_changed (pc->hadj);
            gtk_adjustment_value_changed (pc->vadj);
        }
        previewgui_drawarea_resize (pc);
        gtk_widget_queue_draw (pc->drawarea);

        return TRUE;
    }
    return FALSE;
}

gboolean on_key_press (GtkWidget* w, GdkEventButton* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    pc->prev_x = e->x;
    pc->prev_y = e->y;
    return FALSE;
}

gboolean on_motion (GtkWidget* w, GdkEventMotion* e, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    gdouble delta_x = 0, delta_y = 0, current_x = 0, current_y = 0;

    delta_x = e->x - pc->prev_x;
    delta_y = e->y - pc->prev_y;

    current_x = gtk_adjustment_get_value (pc->hadj);
    current_y = gtk_adjustment_get_value (pc->vadj);

    if (current_x - delta_x < gtk_adjustment_get_upper (pc->hadj) -
                              pc->scrollw->allocation.width) {
        gtk_adjustment_set_value (pc->hadj, current_x - delta_x);
        gtk_adjustment_value_changed (pc->hadj);
    }
    if (current_y - delta_y < gtk_adjustment_get_upper (pc->vadj) -
                              pc->scrollw->allocation.height) {
        gtk_adjustment_set_value (pc->vadj, current_y - delta_y);
        gtk_adjustment_value_changed (pc->vadj);
    }

    return TRUE;
}

gboolean on_resize (GtkWidget* w, GdkRectangle* r, void* user) {
    GuPreviewGui* pc = GU_PREVIEW_GUI(user);
    list_sizes[0] = r->height / pc->page_height;
    list_sizes[1] = r->width / pc->page_width;
    return FALSE;
}
