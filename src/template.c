/**
 * @file   template.c
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

#include "template.h"

#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "configfile.h"
#include "environment.h"
#include "utils.h"


GuTemplate* template_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GuTemplate* t = (GuTemplate*)g_malloc(sizeof(GuTemplate));
    t->templatewindow =
        GTK_WINDOW(gtk_builder_get_object(builder, "templatewindow"));
    t->templateview = 
        GTK_TREE_VIEW(gtk_builder_get_object(builder, "template_treeview"));
    t->list_templates =
        GTK_LIST_STORE(gtk_builder_get_object(builder, "list_templates"));
    // TODO: Setting this from Glade file doesn't work - fix this. 
    GtkCellRendererText* ren;
    ren = GTK_CELL_RENDERER_TEXT(gtk_builder_get_object(builder, "template_renderer"));
    g_object_set(ren, "editable", TRUE, NULL);
    return t;
}

void template_setup(GuTemplate* t) {
    const gchar *filename;
    GError *error = NULL;
    GtkTreeIter iter;
    const char *dirpath = g_build_filename
                (g_get_user_config_dir(), "gummi", "templates" , NULL);
    
    GDir* dir = g_dir_open (dirpath, 0, &error);    
    if (error) { // print error if directory does not exist
        slog(L_G_ERROR, "unable to open/read template directory!\n");
        return;
    }

    while ( (filename = g_dir_read_name(dir))) {
        char *filepath;
        filepath = g_build_filename(dirpath, filename, NULL);
        gtk_list_store_append(t->list_templates, &iter);
        gtk_list_store_set(t->list_templates, &iter, 0, filename, 1, filepath, -1);
    }
}

gchar* template_open_selected(GuTemplate* t) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *filepath;
    
    model = gtk_tree_view_get_model(t->templateview);
    selection = gtk_tree_view_get_selection(t->templateview);
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, 1, &filepath, -1);
    }

	gchar *data;
	g_file_get_contents(filepath, &data, NULL, NULL);
    return data;
}


void template_add_new_entry(GuTemplate* t, gchar* doc) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeViewColumn *col;
    GList *cells;
    
    gtk_list_store_append(t->list_templates, &iter);
    
    col = gtk_tree_view_get_column(t->templateview, 0);
    model = gtk_tree_view_get_model(t->templateview);
    path = gtk_tree_model_get_path(model, &iter);
    cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT(col));

    gtk_tree_view_set_cursor_on_cell
        (t->templateview, path, col, cells->data, TRUE);
}

void template_remove_entry(GuTemplate* t) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *filepath;

    model = gtk_tree_view_get_model(t->templateview);
    selection = gtk_tree_view_get_selection(t->templateview);
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, 1, &filepath, -1);
        gtk_list_store_remove(t->list_templates, &iter);
        g_remove(filepath);
    }
}




