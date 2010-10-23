/**
 * @file   preview.c
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

#include "preview.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include <poppler.h> 
#include <math.h>

#include "environment.h"
#include "utils.h"

GuPreview* preview_init(GtkBuilder * builder, GuFileInfo* finfo) {
    L_F_DEBUG;
    GuPreview* p = g_new0(GuPreview, 1);
    GdkColor bg = {0,0xed00,0xec00,0xeb00};
    p->b_finfo = finfo;
    p->preview_viewport =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "preview_view"));
    p->drawarea = GTK_WIDGET(gtk_builder_get_object(builder, "preview_draw"));
    p->scrollw = GTK_WIDGET(gtk_builder_get_object(builder, "preview_scroll"));
    p->page_next = GTK_WIDGET(gtk_builder_get_object(builder, "page_next"));
    p->page_prev = GTK_WIDGET(gtk_builder_get_object(builder, "page_prev"));
    p->page_label = GTK_WIDGET(gtk_builder_get_object(builder, "page_label"));
    p->page_input = GTK_WIDGET(gtk_builder_get_object(builder, "page_input"));
    p->uri = NULL;
    p->doc = NULL;
    p->page = NULL;
    p->fit_width = TRUE;
    p->best_fit = FALSE;
    gtk_widget_modify_bg(p->drawarea, GTK_STATE_NORMAL, &bg); 

    g_signal_connect(GTK_OBJECT(p->drawarea), "expose-event",
            G_CALLBACK(on_expose), p); 

    slog(L_INFO, "using libpoppler %s ...\n", poppler_get_version());
    return p;
}

void preview_set_pdffile(GuPreview* pc, const gchar *pdffile) {
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
    preview_set_pagedata(pc);
}

void preview_refresh(GuPreview* pc) {
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
    preview_set_pagedata(pc);

    gtk_widget_queue_draw(pc->drawarea);
}

void preview_set_pagedata(GuPreview* pc) {
    L_F_DEBUG;
    if ((pc->page_total - 1) > pc->page_current) {
        gtk_widget_set_sensitive(GTK_WIDGET(pc->page_next), TRUE);
    }
    else if (pc->page_current >= pc->page_total) {
        preview_goto_page(pc, pc->page_total -1);
    }
    char* current = g_strdup_printf("%d", (pc->page_current+1));
    char* total = g_strdup_printf("of %d", pc->page_total);

    gtk_entry_set_text(GTK_ENTRY(pc->page_input), current);
    gtk_label_set_text(GTK_LABEL(pc->page_label), total);

    g_free(current);
    g_free(total);
}

void preview_goto_page(GuPreview* pc, int page_number) {
    L_F_DEBUG;
    if (page_number < 0 || page_number >= pc->page_total)
        slog(L_ERROR, "page_number is a negative number!\n");

    pc->page_current = page_number;
    gtk_widget_set_sensitive(pc->page_prev, (page_number > 0));
    gtk_widget_set_sensitive(pc->page_next,
            (page_number < (pc->page_total -1)));
    preview_refresh(pc);
    // set label info
}

gboolean on_expose(GtkWidget* w, GdkEventExpose* e, GuPreview* pc) {
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
