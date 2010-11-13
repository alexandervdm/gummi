/**
 * @file    gui-preview.h
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

#ifndef GUMMI_GUI_PREVIEW_H
#define GUMMI_GUI_PREVIEW_H

#include <gtk/gtk.h>
#include <poppler.h> 

#include "fileinfo.h"

typedef struct _GuPreviewGui {
    PopplerDocument* doc;
    PopplerPage* page;
    GtkViewport* previewgui_viewport;
    GtkWidget* statuslight;
    GtkWidget* drawarea;
    GtkWidget* page_next;
    GtkWidget* page_prev;
    GtkWidget* page_label;
    GtkWidget* page_input;
    GtkWidget* scrollw;
    GtkLabel* errorlabel;

    gchar *uri;
    gint page_total;
    gint page_current;
    gdouble page_scale;
    gdouble page_width;
    gdouble page_height;
    gdouble page_ratio;
    gboolean fit_width;
    gboolean best_fit;
    gboolean errormode;

    gint update_timer;
    gboolean preview_on_idle;
} GuPreviewGui;

GuPreviewGui* previewgui_init(GtkBuilder * builder);
void previewgui_update_statuslight(const gchar* type);
void previewgui_set_pdffile(GuPreviewGui* prev, const gchar *pdffile);
void previewgui_refresh(GuPreviewGui* prev);
void previewgui_set_pagedata(GuPreviewGui* prev);
void previewgui_goto_page(GuPreviewGui* prev, int page_number);
void previewgui_start_error_mode(GuPreviewGui* pc);
void previewgui_stop_error_mode(GuPreviewGui* pc);
void previewgui_reset(GuPreviewGui* pc);
gboolean previewgui_update_preview(gpointer user);
void previewgui_start_preview(GuPreviewGui* pc);
void previewgui_stop_preview(GuPreviewGui* pc);

gboolean on_expose(GtkWidget* w, GdkEventExpose* e, GuPreviewGui* prev);

#endif /* GUMMI_GUI_PREVIEW_H */
