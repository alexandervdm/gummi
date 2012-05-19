/**
 * @file   gui-preview.h
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

#ifndef __GUMMI_GUI_PREVIEW_H__
#define __GUMMI_GUI_PREVIEW_H__

#include <gtk/gtk.h>
#include <poppler.h> 

#define PAGE_MARGIN 14
#define DOCUMENT_MARGIN (PAGE_MARGIN/2)
#define PAGE_SHADOW_WIDTH 4
#define PAGE_SHADOW_OFFSET 4

#define ASCROLL_STEPS 25
#define ASCROLL_CONST_C (1.5)
#define ASCROLL_CONST_B (-2*ASCROLL_CONST_C + 5./2)
#define ASCROLL_CONST_A (ASCROLL_CONST_C - 3./2)


#define BYTES_PER_PIXEL 4

/**
 *  These "Layered" Rectangles are just like normal GdkRectangles, except the 
 *  have a layer assigned. 2 Rectangles can only intersect or be unioned if they
 *  are on the same layer.
 * 
 *  In the future these structs should be used to describe the position of a 
 *  page. In continuous mode all pages are on the same layer (0). In Paged mode, 
 *  they are on different layers. The field of view (fov) rectangle, that 
 *  describes the portion of the preview that is currently visible is on the 
 *  layer/page that should currently be displayed (0 in continuous mode).
 */
typedef struct _LayeredRectangle LayeredRectangle;
struct _LayeredRectangle {
    gint x;
    gint y;
    gint width;
    gint height;
    gint layer;
};

enum GuPreviewFitMode {
    FIT_NONE = 0, 
    FIT_WIDTH, 
    FIT_HEIGHT, 
    FIT_BOTH
};

#define GU_PREVIEW_PAGE(x) ((GuPreviewPage*)(x))
typedef struct _GuPreviewPage GuPreviewPage;

struct _GuPreviewPage {
    cairo_surface_t* rendering;
    
    double height;
    double width;
    
    LayeredRectangle inner; // Position of the page itself
    LayeredRectangle outer; // Position of the page + border & shadow
    
};

#define GU_PREVIEW_GUI(x) ((GuPreviewGui*)x)
typedef struct _GuPreviewGui GuPreviewGui;

struct _GuPreviewGui {
    PopplerDocument* doc;
    GtkViewport* previewgui_viewport;
    GtkWidget* previewgui_toolbar;
    GtkWidget* statuslight;
    GtkWidget* drawarea;
    GtkWidget* page_next;
    GtkWidget* page_prev;
    GtkWidget* page_label;
    GtkWidget* page_input;
    GtkWidget* scrollw;
    GtkWidget* errorpanel;
    GtkComboBox* combo_sizes;

    gulong page_input_changed_handler;
    gulong combo_sizes_changed_handler;
    gulong on_resize_handler;
    gulong on_expose_handler;
    gulong hvalue_changed_handler;
    gulong vvalue_changed_handler;
    gulong hchanged_handler;
    gulong vchanged_handler;

    GtkRadioMenuItem *page_layout_single_page;
    GtkRadioMenuItem *page_layout_one_column;

    gchar *uri;
    gint update_timer;
    gboolean preview_on_idle;

    GtkAdjustment* hadj;
    GtkAdjustment* vadj;
    gdouble prev_x;
    gdouble prev_y;
    gdouble restore_x;
    gdouble restore_y;

    gint n_pages;
    gint current_page;
    gdouble max_page_height;
    gdouble height_pages;
    gdouble width_pages;
    gdouble height_scaled;
    gdouble width_scaled;
    gdouble width_left;
    gdouble width_no_scale;
    gdouble scale;
    PopplerPageLayout pageLayout;
    GuPreviewPage *pages;
    enum GuPreviewFitMode fit_mode;
    gint cache_size;
    
    gint document_width_scaling;
    gint document_height_scaling;
    gint document_width_non_scaling;
    gint document_height_non_scaling;

    gint next_page;
    gint prev_page;

    gint ascroll_steps_left;
    gint ascroll_end_x;
    gint ascroll_end_y;
    gint ascroll_dist_x;
    gint ascroll_dist_y;

    GSList *sync_nodes;
};

GuPreviewGui* previewgui_init (GtkBuilder * builder);
void previewgui_update_statuslight (const gchar* type);
void previewgui_set_pdffile (GuPreviewGui* prev, const gchar *uri);
void previewgui_refresh (GuPreviewGui* prev, GtkTextIter *sync_to, gchar* tex_file);
void previewgui_set_pagedata (GuPreviewGui* prev);
void previewgui_goto_page (GuPreviewGui* prev, int page_number);
void previewgui_scroll_to_page (GuPreviewGui* pc, int page);
void previewgui_goto_xy (GuPreviewGui* pc, gdouble x, gdouble y);
void previewgui_scroll_to_xy (GuPreviewGui* pc, gdouble x, gdouble y);
void previewgui_save_position (GuPreviewGui* pc);
void previewgui_restore_position (GuPreviewGui* pc);
void previewgui_reset (GuPreviewGui* pc);
void previewgui_cleanup_fds (GuPreviewGui* pc);
void previewgui_start_preview (GuPreviewGui* pc);
void previewgui_drawarea_resize (GuPreviewGui* pc);
void previewgui_stop_preview (GuPreviewGui* pc);
void on_page_input_changed (GtkEntry* entry, void* user);
void on_next_page_clicked (GtkWidget* widget, void* user);
void on_prev_page_clicked (GtkWidget* widget, void* user);
void on_combo_sizes_changed (GtkWidget* widget, void* user);
gboolean on_expose (GtkWidget* w, GdkEventExpose* e, void* user);
gboolean on_scroll (GtkWidget* w, GdkEventScroll* e, void* user);
gboolean on_motion (GtkWidget* w, GdkEventMotion* e, void* user);
gboolean on_resize (GtkWidget* w, GdkRectangle* r, void* user);
gboolean on_scroll_child(GtkScrolledWindow *scrolledwindow, GtkScrollType type,
                         gboolean isHorizontal, gpointer *user);
void on_adj_changed(GtkAdjustment *adjustment, gpointer user);
void previewgui_page_layout_radio_changed(GtkMenuItem *radioitem,gpointer data);
void previewgui_set_page_layout(GuPreviewGui* pc, PopplerPageLayout pageLayout);

gboolean run_garbage_collector(GuPreviewGui* pc);


#endif /* __GUMMI_GUI_PREVIEW_H__ */
