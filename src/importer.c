/**
 * @file   importer.c
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

#include "importer.h"

#include <string.h>

#include <glib.h>

#include "editor.h"
#include "environment.h"
#include "utils.h"

const gchar align_type[][4] = { "l", "c", "r" };
const gchar bracket_type[][16] = { "matrix", "pmatrix", "bmatrix",
                                  "Bmatrix", "vmatrix", "Vmatrix" };

GuImporter* importer_init(GtkBuilder* builder, GuFileInfo* finfo) {
    L_F_DEBUG;
    GuImporter* i = g_new0(GuImporter, 1);

    i->b_finfo = finfo;
    i->import_tabs =
        GTK_NOTEBOOK(gtk_builder_get_object(builder, "import_tabs"));

    i->image_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "image_pane"));
    i->image_file =
        GTK_ENTRY(gtk_builder_get_object(builder, "image_file"));
    i->image_caption =
        GTK_ENTRY(gtk_builder_get_object(builder, "image_caption"));
    i->image_label =
        GTK_ENTRY(gtk_builder_get_object(builder, "image_label"));
    i->image_scale =
        GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "image_scale"));
    i->scaler =
        GTK_ADJUSTMENT(gtk_builder_get_object(builder, "image_scaler"));

    i->table_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "table_pane"));
    i->table_comboalign =
        GTK_COMBO_BOX(gtk_builder_get_object(builder, "table_comboalign"));
    i->table_comboborder =
        GTK_COMBO_BOX(gtk_builder_get_object(builder, "table_comboborder"));
    i->table_rows =
        GTK_ADJUSTMENT(gtk_builder_get_object(builder, "table_rows"));
    i->table_cols =
        GTK_ADJUSTMENT(gtk_builder_get_object(builder, "table_cols"));

    i->matrix_rows =
        GTK_ADJUSTMENT(gtk_builder_get_object(builder, "matrix_rows"));
    i->matrix_cols =
        GTK_ADJUSTMENT(gtk_builder_get_object(builder, "matrix_cols"));
    i->matrix_combobracket =
        GTK_COMBO_BOX(gtk_builder_get_object(builder,"matrix_combobracket"));

    gtk_adjustment_set_value(i->table_cols, 3);
    gtk_adjustment_set_value(i->table_rows, 3);
    gtk_adjustment_set_value(i->matrix_cols, 3);
    gtk_adjustment_set_value(i->matrix_rows, 3);
    return i;
}

void importer_insert_table(GuImporter* ic, GuEditor* ec) {
    L_F_DEBUG;
    GtkTextIter current;
    const gchar* text = importer_generate_table(ic);
    editor_get_current_iter(ec, &current);
    gtk_text_buffer_begin_user_action(ec_sourcebuffer);
    gtk_text_buffer_insert(ec_sourcebuffer, &current, text, strlen(text));
    gtk_text_buffer_end_user_action(ec_sourcebuffer);
    gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
    gtk_notebook_set_current_page(ic->import_tabs, 0);
}

void importer_insert_matrix(GuImporter* ic, GuEditor* ec) {
    L_F_DEBUG;
    GtkTextIter current;
    const gchar* text = importer_generate_matrix(ic);
    editor_insert_package(ec, "amsmath");
    editor_get_current_iter(ec, &current);
    gtk_text_buffer_begin_user_action(ec_sourcebuffer);
    gtk_text_buffer_insert(ec_sourcebuffer, &current, text, strlen(text));
    gtk_text_buffer_end_user_action(ec_sourcebuffer);
    gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
    gtk_notebook_set_current_page(ic->import_tabs, 0);
}

void importer_insert_image(GuImporter* ic, GuEditor* ec) {
    L_F_DEBUG;
    GtkTextIter current;
    const gchar* text = importer_generate_image(ic);
    const gchar* imagefile = gtk_entry_get_text(ic->image_file);

    if (0 != strlen(imagefile)) {
        if (!utils_path_exists(imagefile)) {
            slog(L_G_ERROR, _("%s: No such file or directory\n"), imagefile);
        } else {
            editor_insert_package(ec, "graphicx");
            editor_get_current_iter(ec, &current);
            gtk_text_buffer_begin_user_action(ec_sourcebuffer);
            gtk_text_buffer_insert(ec_sourcebuffer, &current,text,strlen(text));
            gtk_text_buffer_end_user_action(ec_sourcebuffer);
            gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
            importer_imagegui_set_sensitive(ic, "", FALSE);
        }
    }
    gtk_notebook_set_current_page(ic->import_tabs, 0);
}

void importer_imagegui_set_sensitive(GuImporter* ic, const gchar* name,
       gboolean mode) {
    gtk_widget_set_sensitive(GTK_WIDGET(ic->image_label), mode);
    gtk_widget_set_sensitive(GTK_WIDGET(ic->image_caption), mode);
    gtk_widget_set_sensitive(GTK_WIDGET(ic->image_scale), mode);
    gtk_entry_set_text(ic->image_file, name);
    gtk_entry_set_text(ic->image_label, "");
    gtk_entry_set_text(ic->image_caption, "");
    gtk_adjustment_set_value(ic->scaler, 1.00);
}

const gchar* importer_generate_table(GuImporter* ic) {
    L_F_DEBUG;
    gint i = 0, j = 0;
    static gchar result[BUFSIZ * 2] = { 0 };
    gchar table[BUFSIZ * 2] = { 0 },
          begin_tabular[BUFSIZ] = "\\begin{tabular}{", 
          end_tabular[] = "\n\\end{tabular}\n", 
          line[] = "\n\\hline",
          tmp[BUFSIZ / 8];
    gint rows = gtk_adjustment_get_value(ic->table_rows);
    gint cols = gtk_adjustment_get_value(ic->table_cols);
    gint borders = gtk_combo_box_get_active(ic->table_comboborder);
    gint alignment = gtk_combo_box_get_active(ic->table_comboalign);

    /* clear previous data */
    result[0] = 0;

    if (borders)
        strncat(begin_tabular, "|", BUFSIZ - strlen(begin_tabular) -1);
    for (i = 0; i < cols; ++i) {
        strncat(begin_tabular, align_type[alignment], BUFSIZ
                -strlen(begin_tabular) -1);
        if (borders == 2 || (borders == 1 && i == cols -1))
            strncat(begin_tabular, "|", BUFSIZ -strlen(begin_tabular) -1);
    }
    strncat(begin_tabular, "}", BUFSIZ -strlen(begin_tabular) -1);
    if (borders)
        strncat(table, line, BUFSIZ * 2 -strlen(table) -1);
    for (i = 0; i < rows; ++i) {
        strncat(table, "\n\t", BUFSIZ * 2 -strlen(table) -1);
        for (j = 0; j < cols; ++j) {
            snprintf(tmp, BUFSIZ/8, "%d%d", i + 1, j + 1);
            strncat(table, tmp, BUFSIZ * 2 -strlen(table) -1);
            if (j != cols -1)
                strncat(table, " & ", BUFSIZ * 2 -strlen(table) -1);
            else
                strncat(table, "\\\\", BUFSIZ * 2 -strlen(table) -1);
        }
        if (borders == 2 || (borders == 1 && i == rows -1))
            strncat(table, line, BUFSIZ * 2 -strlen(table) -1);
    }
    strncat(result, begin_tabular, BUFSIZ *2 -strlen(result) -1);
    strncat(result, table, BUFSIZ *2 -strlen(result) -1);
    strncat(result, end_tabular, BUFSIZ *2 -strlen(result) -1);
    return result;
}

const gchar* importer_generate_matrix(GuImporter* ic) {
    L_F_DEBUG;
    gint i = 0, j = 0;
    static gchar result[BUFSIZ * 2] = { 0 };
    gchar tmp[BUFSIZ / 8];
    gint bracket = gtk_combo_box_get_active(ic->matrix_combobracket);
    gint rows = gtk_adjustment_get_value(ic->matrix_rows);
    gint cols = gtk_adjustment_get_value(ic->matrix_cols);

    /* clear previous data */
    result[0] = 0;

    strncat(result, "$\\begin{", BUFSIZ * 2 -strlen(result) -1);
    strncat(result, bracket_type[bracket], BUFSIZ * 2 -strlen(result) -1);
    strncat(result, "}", BUFSIZ * 2 - strlen(result) -1);

    for (i = 0; i < rows; ++i) {
        strncat(result, "\n\t", BUFSIZ * 2 -strlen(result) -1);
        for (j = 0; j < cols; ++j) {
            snprintf(tmp, BUFSIZ/8, "%d%d", i + 1, j + 1);
            strncat(result, tmp, BUFSIZ * 2 -strlen(result) -1);
            if (j != cols -1)
                strncat(result, " & ", BUFSIZ * 2 -strlen(result) -1);
            else
                strncat(result, "\\\\", BUFSIZ * 2 -strlen(result) -1);
        }
    }
    strncat(result, "\n\\end{", BUFSIZ * 2 -strlen(result) -1);
    strncat(result, bracket_type[bracket], BUFSIZ * 2 -strlen(result) -1);
    strncat(result, "}$\n", BUFSIZ * 2 -strlen(result) -1);
    return result;
}

const gchar* importer_generate_image(GuImporter* ic) {
    L_F_DEBUG;
    const gchar* image_file = gtk_entry_get_text(ic->image_file);
    const gchar* caption = gtk_entry_get_text(ic->image_caption);
    const gchar* label = gtk_entry_get_text(ic->image_label);
    gchar* root_path = NULL;
    gchar* relative_path = NULL;
    gdouble scale = gtk_adjustment_get_value(ic->scaler);
    static gchar result[BUFSIZ] = { 0 };

    /* clear previous data */
    result[0] = 0;

    if (ic->b_finfo->filename)
        root_path = g_path_get_dirname(ic->b_finfo->filename);
    relative_path = utils_path_to_relative(root_path, image_file);

    snprintf(result, BUFSIZ, "\\begin{figure}[htp]\n\\centering\n"
        "\\includegraphics[scale=%.2f]{%s}\n\\caption{%s}\n\\label{%s}\n"
        "\\end{figure}", scale, relative_path, caption, label);

    g_free(relative_path);
    g_free(root_path);
    return result;
}
