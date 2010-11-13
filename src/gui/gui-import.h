/**
 * @file   gui-import.h
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

#ifndef GUMMI_GUI_IMPORT_H
#define GUMMI_GUI_IMPORT_H

#include <glib.h>
#include <gtk/gtk.h>

#define g_importgui gui->importgui

typedef struct _GuImportGui {
    GtkHBox* box_image;
    GtkHBox* box_table;
    GtkHBox* box_matrix;

    GtkNotebook* import_tabs;
    GtkViewport* image_pane;
    GtkViewport* table_pane;
    GtkViewport* matrix_pane;

    GtkEntry* image_file;
    GtkEntry* image_caption;
    GtkEntry* image_label;
    GtkSpinButton* image_scale;
    GtkAdjustment* scaler;

    GtkComboBox* table_comboalign;
    GtkComboBox* table_comboborder;
    GtkAdjustment* table_rows;
    GtkAdjustment* table_cols;

    GtkAdjustment* matrix_rows;
    GtkAdjustment* matrix_cols;
    GtkComboBox* matrix_combobracket;
} GuImportGui;

GuImportGui* importgui_init(GtkBuilder* builder);
void on_import_tabs_switch_page(GtkNotebook* notebook, GtkNotebookPage* page,
        guint page_num, void* user);
void on_button_import_table_apply_clicked(GtkWidget* widget, void* user);
void on_button_import_image_apply_clicked(GtkWidget* widget, void* user);
void on_button_import_matrix_apply_clicked(GtkWidget* widget, void* user);
void on_image_file_activate(void);
void importer_imagegui_set_sensitive(const gchar* name, gboolean mode);

#endif /* GUMMI_GUI_IMPORT_H */
