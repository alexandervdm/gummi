/**
 * @file   gui-import.c
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

#include "gui-import.h"

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "environment.h"
#include "utils.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuImportGui* importgui_init(GtkBuilder* builder) {
    g_return_val_if_fail(GTK_IS_BUILDER(builder), NULL);

    GuImportGui* i = g_new0(GuImportGui, 1);
    i->box_image =
        GTK_HBOX(gtk_builder_get_object(builder, "box_image"));
    i->box_table =
        GTK_HBOX(gtk_builder_get_object(builder, "box_table"));
    i->box_matrix =
        GTK_HBOX(gtk_builder_get_object(builder, "box_matrix"));

    i->import_tabs =
        GTK_NOTEBOOK(gtk_builder_get_object(builder, "import_tabs"));
    i->image_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "image_pane"));
    i->table_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "table_pane"));
    i->matrix_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "matrix_pane"));

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

void on_import_tabs_switch_page(GtkNotebook* notebook, GtkNotebookPage* page,
        guint page_num, void* user) {
    GList* list = NULL;
    list = gtk_container_get_children(
            GTK_CONTAINER(g_importgui->box_image));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(g_importgui->box_image),
                GTK_WIDGET(list->data));
        list = list->next;
    }
    list = gtk_container_get_children(
            GTK_CONTAINER(g_importgui->box_table));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(g_importgui->box_table),
                GTK_WIDGET(list->data));
        list = list->next;
    }
    list = gtk_container_get_children(
            GTK_CONTAINER(g_importgui->box_matrix));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(g_importgui->box_matrix),
                GTK_WIDGET(list->data));
        list = list->next;
    }

    switch (page_num) {
        case 1:
            gtk_container_add(GTK_CONTAINER(g_importgui->box_image),
                    GTK_WIDGET(g_importgui->image_pane));
            break;
        case 2:
            gtk_container_add(GTK_CONTAINER(g_importgui->box_table),
                    GTK_WIDGET(g_importgui->table_pane));
            break;
        case 3:
            gtk_container_add(GTK_CONTAINER(g_importgui->box_matrix),
                    GTK_WIDGET(g_importgui->matrix_pane));
            break;
    }
}

void on_button_import_table_apply_clicked(GtkWidget* widget, void* user) {
    GtkTextIter current;
    gint rows = gtk_adjustment_get_value(g_importgui->table_rows);
    gint cols = gtk_adjustment_get_value(g_importgui->table_cols);
    gint border = gtk_combo_box_get_active(g_importgui->table_comboborder);
    gint align = gtk_combo_box_get_active(g_importgui->table_comboalign);
    const gchar* text = importer_generate_table(rows, cols, border, align);

    editor_get_current_iter(gummi->editor, &current);
    gtk_text_buffer_begin_user_action(g_e_buffer);
    gtk_text_buffer_insert(g_e_buffer, &current, text, strlen(text));
    gtk_text_buffer_end_user_action(g_e_buffer);
    gtk_text_buffer_set_modified(g_e_buffer, TRUE);
    gtk_notebook_set_current_page(g_importgui->import_tabs, 0);
}

void on_button_import_image_apply_clicked(GtkWidget* widget, void* user) {
    GtkTextIter current;
    const gchar* imagefile = gtk_entry_get_text(g_importgui->image_file);
    const gchar* caption = gtk_entry_get_text(g_importgui->image_caption);
    const gchar* label = gtk_entry_get_text(g_importgui->image_label);
    gdouble scale = gtk_adjustment_get_value(g_importgui->scaler);
    gchar* root_path = NULL;
    gchar* relative_path = NULL;
    const gchar* text = 0;

    if (0 != strlen(imagefile)) {
        if (!utils_path_exists(imagefile)) {
            slog(L_G_ERROR, _("%s: No such file or directory\n"), imagefile);
        } else {
            if (gummi->finfo->filename)
                root_path = g_path_get_dirname(gummi->finfo->filename);
            relative_path = utils_path_to_relative(root_path, imagefile);
            text =importer_generate_image(relative_path, caption, label, scale);
            editor_insert_package(gummi->editor, "graphicx");
            editor_get_current_iter(gummi->editor, &current);
            gtk_text_buffer_begin_user_action(g_e_buffer);
            gtk_text_buffer_insert(g_e_buffer, &current,text,strlen(text));
            gtk_text_buffer_end_user_action(g_e_buffer);
            gtk_text_buffer_set_modified(g_e_buffer, TRUE);
            importer_imagegui_set_sensitive("", FALSE);
        }
    }
    gtk_notebook_set_current_page(g_importgui->import_tabs, 0);

    g_free(relative_path);
    g_free(root_path);
}

void on_button_import_matrix_apply_clicked(GtkWidget* widget, void* user) {
    GtkTextIter current;
    gint bracket =
        gtk_combo_box_get_active(g_importgui->matrix_combobracket);
    gint rows = gtk_adjustment_get_value(g_importgui->matrix_rows);
    gint cols = gtk_adjustment_get_value(g_importgui->matrix_cols);
    const gchar* text = importer_generate_matrix(bracket, rows, cols);
    editor_insert_package(gummi->editor, "amsmath");
    editor_get_current_iter(gummi->editor, &current);
    gtk_text_buffer_begin_user_action(g_e_buffer);
    gtk_text_buffer_insert(g_e_buffer, &current, text, strlen(text));
    gtk_text_buffer_end_user_action(g_e_buffer);
    gtk_text_buffer_set_modified(g_e_buffer, TRUE);
    gtk_notebook_set_current_page(g_importgui->import_tabs, 0);
}

void on_image_file_activate(void) {
    gchar* filename = get_open_filename(TYPE_IMAGE);
    importer_imagegui_set_sensitive(filename, TRUE);
    g_free(filename);
}

void importer_imagegui_set_sensitive(const gchar* name, gboolean mode) {
    gtk_widget_set_sensitive(GTK_WIDGET(g_importgui->image_label), mode);
    gtk_widget_set_sensitive(GTK_WIDGET(g_importgui->image_caption), mode);
    gtk_widget_set_sensitive(GTK_WIDGET(g_importgui->image_scale), mode);
    gtk_entry_set_text(g_importgui->image_file, name);
    gtk_entry_set_text(g_importgui->image_label, "");
    gtk_entry_set_text(g_importgui->image_caption, "");
    gtk_adjustment_set_value(g_importgui->scaler, 1.00);
}
