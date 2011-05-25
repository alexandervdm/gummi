/**
 * @file   gui-preview.h
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

#ifndef __GUMMI_GUI_PREVIEW_H__
#define __GUMMI_GUI_PREVIEW_H__

#include <gtk/gtk.h>
#include <poppler.h> 

#define PAGE_MARGIN 50
#define SIZE_COUNT 9

#define GU_PREVIEW_GUI(x) ((GuPreviewGui*)x)
typedef struct _GuPreviewGui GuPreviewGui;

struct _GuPreviewGui {
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
    GtkWidget* errorpanel;
    GtkComboBox* combo_sizes;

    gchar *uri;
    gint update_timer;
    gint page_total;
    gint page_current;
    gint page_zoommode;
    gdouble page_scale;
    gdouble page_width;
    gdouble page_height;
    gdouble page_ratio;
    gboolean preview_on_idle;
    gboolean errormode;
    
    cairo_surface_t *surface;
    GtkAdjustment* hadj;
    GtkAdjustment* vadj;
    gdouble prev_x;
    gdouble prev_y;
};

GuPreviewGui* previewgui_init (GtkBuilder * builder);
void previewgui_update_statuslight (const gchar* type);
void previewgui_set_pdffile (GuPreviewGui* prev, const gchar *pdffile);
void previewgui_refresh (GuPreviewGui* prev);
void previewgui_set_pagedata (GuPreviewGui* prev);
void previewgui_goto_page (GuPreviewGui* prev, int page_number);
void previewgui_start_error_mode (GuPreviewGui* pc);
void previewgui_stop_error_mode (GuPreviewGui* pc);
void previewgui_reset (GuPreviewGui* pc);
void previewgui_cleanup_fds (GuPreviewGui* pc);
void previewgui_start_preview (GuPreviewGui* pc);
void previewgui_drawarea_resize (GuPreviewGui* pc);
void previewgui_stop_preview (GuPreviewGui* pc);
void previewgui_page_input_changed (GtkEntry* entry, void* user);
void previewgui_next_page (GtkWidget* widget, void* user);
void previewgui_prev_page (GtkWidget* widget, void* user);
void previewgui_zoom_change (GtkWidget* widget, void* user);
gboolean on_expose (GtkWidget* w, GdkEventExpose* e, void* user);
gboolean on_scroll (GtkWidget* w, GdkEventScroll* e, void* user);
gboolean on_motion (GtkWidget* w, GdkEventMotion* e, void* user);
gboolean on_resize (GtkWidget* w, GdkRectangle* r, void* user);
gboolean on_key_press (GtkWidget* w, GdkEventButton* e, void* user);

#endif /* __GUMMI_GUI_PREVIEW_H__ */
