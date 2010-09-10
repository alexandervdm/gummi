/**
 * @file    preview.h
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

#ifndef GUMMI_PREVIEW_H
#define GUMMI_PREVIEW_H

#include <gtk/gtk.h>
#include <poppler.h> 

typedef struct _GuPreview {
    PopplerDocument* doc;
    PopplerPage* page;
    GtkViewport* preview_viewport;
    GtkWidget *drawarea;
    GtkWidget *page_next;
    GtkWidget *page_prev;
    GtkWidget *page_label;
    GtkWidget *page_input;
    GtkWidget *scrollw;

    gchar *uri;

    gint page_total;
    gint page_current;
    gdouble page_scale;
    gdouble page_width;
    gdouble page_height;
    gdouble page_ratio;
    gboolean fit_width;
    gboolean best_fit;
} GuPreview;

GuPreview* preview_init(GtkBuilder * builder);
void preview_set_pdffile(GuPreview* prev, const gchar *pdffile);
void preview_refresh(GuPreview* prev);
void preview_set_pagedata(GuPreview* prev);
void preview_goto_page(GuPreview* prev, int page_number);

gboolean on_expose(GtkWidget* w, GdkEventExpose* e, GuPreview* prev);

#endif /* GUMMI_PREVIEW_H */
