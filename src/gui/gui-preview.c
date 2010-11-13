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
#include "utils.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuPreviewGui* previewgui_init(GtkBuilder * builder) {
    L_F_DEBUG;
    GuPreviewGui* p = g_new0(GuPreviewGui, 1);
    GdkColor bg = {0, 0xed00, 0xec00, 0xeb00};
    p->previewgui_viewport =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "previewgui_view"));
    p->statuslight =
        GTK_WIDGET(gtk_builder_get_object(builder, "tool_statuslight"));
    p->drawarea =
        GTK_WIDGET(gtk_builder_get_object(builder, "previewgui_draw"));
    p->scrollw =
        GTK_WIDGET(gtk_builder_get_object(builder, "previewgui_scroll"));
    p->page_next = GTK_WIDGET(gtk_builder_get_object(builder, "page_next"));
    p->page_prev = GTK_WIDGET(gtk_builder_get_object(builder, "page_prev"));
    p->page_label = GTK_WIDGET(gtk_builder_get_object(builder, "page_label"));
    p->page_input = GTK_WIDGET(gtk_builder_get_object(builder, "page_input"));
    p->uri = NULL;
    p->doc = NULL;
    p->page = NULL;
    p->fit_width = TRUE;
    p->best_fit = FALSE;
    p->update_timer = 0;
    p->preview_on_idle = FALSE;
    gtk_widget_modify_bg(p->drawarea, GTK_STATE_NORMAL, &bg); 

    g_signal_connect(GTK_OBJECT(p->drawarea), "expose-event",
            G_CALLBACK(on_expose), p); 

    char* message = g_strdup_printf(_("PDF Preview could not initialize.\n\n"
            "It appears your LaTeX document contains errors or\n"
            "the program `%s' was not installed.\n"
            "Additional information is available on the Error Output tab.\n"
            "Please correct the listed errors to restore preview."),
            config_get_value("typesetter"));
    p->errorlabel = GTK_LABEL(gtk_label_new(message));
    gtk_label_set_justify(p->errorlabel, GTK_JUSTIFY_CENTER);
    g_free(message);

    slog(L_INFO, "using libpoppler %s ...\n", poppler_get_version());
    return p;
}

void previewgui_update_statuslight(const gchar* type) {
    gtk_tool_button_set_stock_id(
            GTK_TOOL_BUTTON(gui->previewgui->statuslight), type);
    while (gtk_events_pending()) gtk_main_iteration();
}

void previewgui_set_pdffile(GuPreviewGui* pc, const gchar *pdffile) {
    L_F_DEBUG;
    GError *err = NULL;
    pc->page_current = 0;
    
    g_free(pc->uri);
    pc->uri = g_strconcat("file://", pdffile, NULL);

    /* clean up */
    if (pc->page) g_object_unref(pc->page);
    if (pc->doc) g_object_unref(pc->doc);

    pc->doc = poppler_document_new_from_file(pc->uri, NULL, &err);
    pc->page = poppler_document_get_page(pc->doc, pc->page_current);

    poppler_page_get_size(pc->page, &pc->page_width, &pc->page_height);

    pc->page_total = poppler_document_get_n_pages(pc->doc);
    pc->page_ratio = (pc->page_width / pc->page_height);
    pc->page_scale = 1.0;
    previewgui_set_pagedata(pc);
}

void previewgui_refresh(GuPreviewGui* pc) {
    L_F_DEBUG;
    GError *err = NULL;

    /* This is line is very important, if no pdf exist, preview will fail */
    if (!pc->uri || !utils_path_exists(pc->uri + 7)) return;

    /* clean up */
    if (pc->page) g_object_unref(pc->page);
    if (pc->doc) g_object_unref(pc->doc);

    pc->doc = poppler_document_new_from_file(pc->uri, NULL, &err);
    pc->page = poppler_document_get_page(pc->doc, pc->page_current);
    
    // recheck document dimensions on refresh for orientation changes:
    poppler_page_get_size(pc->page, &pc->page_width, &pc->page_height);  

    pc->page_total = poppler_document_get_n_pages(pc->doc);
    previewgui_set_pagedata(pc);

    gtk_widget_queue_draw(pc->drawarea);
}

void previewgui_set_pagedata(GuPreviewGui* pc) {
    L_F_DEBUG;
    if ((pc->page_total - 1) > pc->page_current) {
        gtk_widget_set_sensitive(GTK_WIDGET(pc->page_next), TRUE);
    }
    else if (pc->page_current >= pc->page_total) {
        previewgui_goto_page(pc, pc->page_total -1);
    }
    char* current = g_strdup_printf("%d", (pc->page_current+1));
    char* total = g_strdup_printf("of %d", pc->page_total);

    gtk_entry_set_text(GTK_ENTRY(pc->page_input), current);
    gtk_label_set_text(GTK_LABEL(pc->page_label), total);

    g_free(current);
    g_free(total);
}

void previewgui_goto_page(GuPreviewGui* pc, int page_number) {
    L_F_DEBUG;
    if (page_number < 0 || page_number >= pc->page_total)
        slog(L_ERROR, "page_number is a negative number!\n");

    pc->page_current = page_number;
    gtk_widget_set_sensitive(pc->page_prev, (page_number > 0));
    gtk_widget_set_sensitive(pc->page_next,
            (page_number < (pc->page_total -1)));
    previewgui_refresh(pc);
    // set label info
}

void previewgui_start_error_mode(GuPreviewGui* pc) {
    L_F_DEBUG;
    pc->errormode = TRUE;
    gtk_container_remove(GTK_CONTAINER(pc->previewgui_viewport),
            GTK_WIDGET(pc->drawarea));
    gtk_container_add(GTK_CONTAINER(pc->previewgui_viewport),
            GTK_WIDGET(pc->errorlabel));
    gtk_widget_show_all(GTK_WIDGET(pc->previewgui_viewport));
}

void previewgui_stop_error_mode(GuPreviewGui* pc) {
    L_F_DEBUG;
    pc->errormode = FALSE;
    g_object_ref(pc->errorlabel);
    gtk_container_remove(GTK_CONTAINER(pc->previewgui_viewport),
            GTK_WIDGET(pc->errorlabel));
    gtk_container_add(GTK_CONTAINER(pc->previewgui_viewport),
            GTK_WIDGET(pc->drawarea));
}

gboolean on_expose(GtkWidget* w, GdkEventExpose* e, GuPreviewGui* pc) {
    L_F_DEBUG;
    /* This line is very important, if no pdf exist, preview fails */
    if (!pc->uri || !utils_path_exists(pc->uri + 7)) return FALSE;

    cairo_t* cr;
    cr = gdk_cairo_create(w->window);
    
    double width = pc->scrollw->allocation.width;
    double height = pc->scrollw->allocation.height;
    double scrollw_ratio = (width / height);
    
    // TODO: STOP WITH ERROR IF PAGE RATIO OR PAGE WIDTH IS NULL!
    
    if (pc->best_fit || pc->fit_width) {
        if (scrollw_ratio < pc->page_ratio || pc->fit_width) {
            pc->page_scale = width / pc->page_width;
        }
        else {
            pc->page_scale = height / pc->page_height;
        }
    }
    
    if (!pc->best_fit && !pc->fit_width) {
        gtk_widget_set_size_request(pc->drawarea, (pc->page_width *
                    pc->page_scale), (pc->page_height * pc->page_scale));
    }
    else if (pc->fit_width) {
        if (fabs(pc->page_ratio - scrollw_ratio) > 0.01) {
            gtk_widget_set_size_request(pc->drawarea, -1,
                    (pc->page_height*pc->page_scale));
        }
    }
    else if (pc->best_fit) {
        gtk_widget_set_size_request(pc->drawarea, -1,
                (pc->page_height*pc->page_scale)-10);
    }
    
    // import python lines for calculating scale here
    cairo_scale(cr, pc->page_scale, pc->page_scale);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, pc->page_width, pc->page_height);
    cairo_fill(cr);
    
    poppler_page_render(pc->page, cr);
    cairo_destroy(cr);
    return FALSE;
} 

void previewgui_reset(GuPreviewGui* pc) {
    L_F_DEBUG;
    gummi->latex->modified_since_compile = TRUE;
    previewgui_stop_preview(pc);
    previewgui_update_preview(pc);

    if (!pc->errormode && gummi->latex->errorline)
        previewgui_start_error_mode(pc);
    else
        previewgui_set_pdffile(pc, gummi->finfo->pdffile);

    if (config_get_value("compile_status"))
        previewgui_start_preview(pc);
}

gboolean previewgui_update_preview(gpointer user) {
    L_F_DEBUG;
    GuPreviewGui* pc = (GuPreviewGui*)user;
    latex_update_workfile(gummi->latex);
    latex_update_pdffile(gummi->latex);
    editor_apply_errortags(gummi->editor, gummi->latex->errorline);
    errorbuffer_set_text(gummi->latex->errormessage);
    if (!gummi->latex->errorline && pc->errormode) {
        previewgui_stop_error_mode(pc);
        previewgui_set_pdffile(pc, gummi->finfo->pdffile);
    }
    previewgui_refresh(pc);
    return (0 == strcmp(config_get_value("compile_scheme"), "real_time"));
}

void previewgui_start_preview(GuPreviewGui* pc) {
    L_F_DEBUG;
    previewgui_stop_preview(pc);
    if (0 == strcmp(config_get_value("compile_scheme"), "on_idle")) {
        pc->preview_on_idle = TRUE;
    } else {
        pc->update_timer = g_timeout_add_seconds(
                atoi(config_get_value("compile_timer")),
                previewgui_update_preview, pc);
    }
}

void previewgui_stop_preview(GuPreviewGui* pc) {
    L_F_DEBUG;
    pc->preview_on_idle = FALSE;
    if (pc->update_timer != 0) g_source_remove(pc->update_timer);
    pc->update_timer = 0;
}

void previewgui_page_input_changed(GtkEntry* entry, void* user) {
    L_F_DEBUG;
    gint newpage = atoi(gtk_entry_get_text(entry));

    if (0 == newpage)
        return;
    else if (newpage >= 1 &&  newpage <= gui->previewgui->page_total) {
        previewgui_goto_page(gui->previewgui, newpage -1);
    } else {
        newpage = CLAMP(newpage, 1, gui->previewgui->page_total);
        gchar* num = g_strdup_printf("%d", newpage);
        gtk_entry_set_text(entry, num);
        g_free(num);
        previewgui_goto_page(gui->previewgui, newpage -1);
    }
}

void previewgui_next_page(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    previewgui_goto_page(gui->previewgui, gui->previewgui->page_current + 1);
}

void previewgui_prev_page(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    previewgui_goto_page(gui->previewgui, gui->previewgui->page_current - 1);
}

void previewgui_zoom_change(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    double opts[9] = {0.50, 0.70, 0.85, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0}; 

    if (index < 0) slog(L_ERROR, "preview zoom level is < 0.\n");

    gui->previewgui->fit_width = gui->previewgui->best_fit = FALSE;
    if (index < 2) {
        if (index == 0) {
            gui->previewgui->best_fit = TRUE;
        }
        else if (index == 1) {
            gui->previewgui->fit_width = TRUE;
        }
    }
    else {
        gui->previewgui->page_scale = opts[index-2];
    }

    gtk_widget_queue_draw(gui->previewgui->drawarea);
}
