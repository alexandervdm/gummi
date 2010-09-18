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
    gchar templatedir[128];
    GError *error = NULL;
    GtkTreeIter iter;

    snprintf(templatedir, 128, "%s%cgummi%ctemplates", 
    g_get_user_config_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);

    GDir* dir = g_dir_open (templatedir, 0, &error);    
    if (error) { // print error if directory does not exist
        slog(L_G_ERROR, "unable to open/read template directory!\n");
        return;
    }

    while ( (filename = g_dir_read_name(dir))) {
        gchar filepath[128];
        snprintf(filepath, 128, "%s%c%s", templatedir, G_DIR_SEPARATOR, filename);
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
    gtk_list_store_append(t->list_templates, &iter);
    
    //gchar *newname = template_iterate_available();
    
    gtk_list_store_set(t->list_templates, &iter, 0, "untitled", -1);
    // sort out config thingies
}

void template_remove_entry(GuTemplate* t) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    
    model = gtk_tree_view_get_model(t->templateview);
    selection = gtk_tree_view_get_selection(t->templateview);
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_list_store_remove(t->list_templates, &iter);
    }
    // sort out config thingies
}


gchar template_iterate_available() {
    // iterate the list, and return next available name
    return "bla";
}



