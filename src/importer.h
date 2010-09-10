/**
 * @file    importer.h
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

#ifndef GUMMI_IMPORTER_H
#define GUMMI_IMPORTER_H

#include <glib.h>
#include <gtk/gtk.h>

#include "editor.h"

typedef struct _GuImporter {
    GtkNotebook* import_tabs;

    GtkViewport* image_pane;
    GtkEntry* image_file;
    GtkEntry* image_caption;
    GtkEntry* image_label;
    GtkSpinButton* image_scale;
    GtkAdjustment* scaler;

    GtkViewport* table_pane;
    GtkComboBox* table_comboalign;
    GtkComboBox* table_comboborder;
    GtkAdjustment* table_rows;
    GtkAdjustment* table_cols;

    GtkAdjustment* matrix_rows;
    GtkAdjustment* matrix_cols;
    GtkComboBox* matrix_combobracket;
} GuImporter;

GuImporter* importer_init(GtkBuilder* builder);
void importer_insert_table(GuImporter* ic, GuEditor* ec);
void importer_insert_matrix(GuImporter* ic, GuEditor* ec);
void importer_insert_image(GuImporter* ic, GuEditor* ec);
void importer_imagegui_set_sensitive(GuImporter* ic, const gchar* name,
        gboolean mode);
const gchar* importer_generate_table(GuImporter* ic);
const gchar* importer_generate_matrix(GuImporter* ic);
const gchar* importer_generate_image(GuImporter* ic);

#endif /* GUMMI_IMPORTER_H */
