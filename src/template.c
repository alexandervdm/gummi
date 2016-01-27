/**
 * @file   template.c
 * @brief
 *
 * Copyright (C) 2009-2016 Gummi Developers
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

#include "template.h"

#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <string.h>

#include "configfile.h"
#include "environment.h"
#include "utils.h"


//TODO: this should really be a gui class in 0.7.0


GuTemplate* template_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);
    GuTemplate* t = g_new0 (GuTemplate, 1);
    t->templatewindow =
        GTK_WINDOW (gtk_builder_get_object (builder, "templatewindow"));
    t->templateview =
        GTK_TREE_VIEW (gtk_builder_get_object (builder, "template_treeview"));
    t->list_templates =
        GTK_LIST_STORE (gtk_builder_get_object (builder, "list_templates"));
    t->template_label =
        GTK_LABEL (gtk_builder_get_object (builder, "template_label"));
    t->template_add =
        GTK_WIDGET (gtk_builder_get_object (builder, "template_add"));
    t->template_remove =
        GTK_WIDGET (gtk_builder_get_object (builder, "template_remove"));
    t->template_open =
        GTK_WIDGET (gtk_builder_get_object (builder, "template_open"));
    t->template_render = GTK_CELL_RENDERER_TEXT (
            gtk_builder_get_object (builder, "template_renderer"));
    t->template_col = GTK_TREE_VIEW_COLUMN (
            gtk_builder_get_object (builder, "template_column"));
    gtk_tree_view_column_set_sort_column_id (t->template_col, 0);
    return t;
}

void template_setup (GuTemplate* t) {
    const gchar *filename;
    char *filepath = NULL;
    GError *error = NULL;
    GtkTreeIter iter;

    gchar *dirpath = g_build_filename (g_get_user_config_dir (), "gummi",
                                      "templates" , NULL);

    GDir* dir = g_dir_open (dirpath, 0, &error);
    if (error) {
        /* print error if directory does not exist */
        slog (L_INFO, "unable to read template directory, creating new..\n");
        g_mkdir_with_parents (dirpath, DIR_PERMS);
        g_free (dirpath);
        return;
    }

    while ( (filename = g_dir_read_name (dir))) {
        filepath = g_build_filename (dirpath, filename, NULL);
        gtk_list_store_append (t->list_templates, &iter);
        gtk_list_store_set (t->list_templates, &iter, 0, filename, 1,
                filepath, -1);
        g_free (filepath);
    }
    g_free (dirpath);

    // disable the add button when there are no tabs opened (#388)
    if (!tabmanager_has_tabs()) {
        gtk_widget_set_sensitive (t->template_add, FALSE);
    }

    gtk_widget_set_sensitive (t->template_open, FALSE);


}

gchar* template_get_selected_path (GuTemplate* t) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *filepath = NULL;

    model = gtk_tree_view_get_model (t->templateview);
    selection = gtk_tree_view_get_selection (t->templateview);

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
        gtk_tree_model_get (model, &iter, 1, &filepath, -1);

    return filepath;
}


void template_add_new_entry (GuTemplate* t) {
    GtkTreeIter iter;
    GtkTreeModel* model = NULL;
    GtkTreePath* path = NULL;
    GtkTreeViewColumn* col = NULL;
    GList* cells = NULL;

    gtk_label_set_text (t->template_label, "");
    gtk_list_store_append (t->list_templates, &iter);

    g_object_set (t->template_render, "editable", TRUE, NULL);
    gtk_widget_set_sensitive (t->template_add, FALSE);
    gtk_widget_set_sensitive (t->template_remove, FALSE);
    gtk_widget_set_sensitive (t->template_open, FALSE);

    col = gtk_tree_view_get_column (t->templateview, 0);
    model = gtk_tree_view_get_model (t->templateview);
    path = gtk_tree_model_get_path (model, &iter);
    cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (col));

    gtk_tree_view_set_cursor_on_cell (t->templateview, path, col, cells->data,
                                     TRUE);
    g_list_free (cells);
    gtk_tree_path_free (path);
}

void template_remove_entry (GuTemplate* t) {
    GtkTreeModel* model = NULL;
    GtkTreeIter iter;
    GtkTreeSelection* selection = NULL;
    gchar* filepath = NULL;

    model = gtk_tree_view_get_model (t->templateview);
    selection = gtk_tree_view_get_selection (t->templateview);

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, 1, &filepath, -1);
        gtk_list_store_remove (t->list_templates, &iter);
        g_remove (filepath);
    }
    gtk_widget_set_sensitive (t->template_open, FALSE);
}

void template_create_file (GuTemplate* t, gchar* filename, gchar* text) {
    const char* filepath = g_build_filename (g_get_user_config_dir (),
            "gummi", "templates", filename, NULL);

    if (g_file_test (filepath, G_FILE_TEST_EXISTS)) {
        gtk_label_set_text (t->template_label, "filename already exists");
        template_remove_entry (t);
    }
    else {
        g_file_set_contents (filepath, text, strlen (text), NULL);
    }
    g_object_set (t->template_render, "editable", FALSE, NULL);
    gtk_widget_set_sensitive (t->template_add, TRUE);
    gtk_widget_set_sensitive (t->template_remove, TRUE);
    gtk_widget_set_sensitive (t->template_open, TRUE);
}

void template_data_free(templdata* data) {
    g_free(data->itemname);
    g_free(data->itemdata);
}
