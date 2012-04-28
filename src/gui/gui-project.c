/**
 * @file    gui-project.c
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

#include "gui-project.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "environment.h"
#include "gui-main.h"
#include "gui-tabmanager.h"
#include "project.h"

extern GummiGui* gui;
extern Gummi* gummi;


GuProjectGui* projectgui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuProjectGui* p = g_new0 (GuProjectGui, 1);
    
    p->proj_name = GTK_LABEL (gtk_builder_get_object (builder, "proj_name"));
    p->proj_path = GTK_LABEL (gtk_builder_get_object (builder, "proj_path"));
    p->proj_nroffiles = 
            GTK_LABEL (gtk_builder_get_object (builder, "proj_nroffiles"));
    
    p->proj_addbutton = 
            GTK_BUTTON (gtk_builder_get_object (builder, "proj_addbutton"));
    p->proj_rembutton = 
            GTK_BUTTON (gtk_builder_get_object (builder, "proj_rembutton"));
    
    p->list_projfiles =
        GTK_LIST_STORE (gtk_builder_get_object (builder, "list_projfiles"));
    p->proj_treeview =
        GTK_TREE_VIEW (gtk_builder_get_object (builder, "proj_treeview"));
        
    return p;
}



void projectgui_set_rootfile (gint position) {
    tabmanagergui_set_current_page (position);
    tablabel_set_bold_text (g_active_tab->page);
}

int projectgui_list_projfiles (gchar* active_proj) {
    gchar* content = NULL;
    GtkTreeIter iter;
    GError* err = NULL;
    GList* files = NULL;
    gint amount, i;
    gchar* name;
    gchar* path;
    
    GtkListStore* store = gui->projectgui->list_projfiles;
    gtk_list_store_clear (store);

    if (!g_file_get_contents (active_proj, &content, NULL, &err)) {
        slog (L_ERROR, "%s\n", err->message);
        return -1;
    }
    
    files = project_list_files (content);
    amount = g_list_length (files);
    
    for (i=0; i < amount; i++) {
        GdkPixbuf* pic = NULL;
        gchar* tmp = g_list_nth_data (files, i);
        name = g_path_get_basename (tmp);
        path = g_path_get_dirname (tmp);
        gtk_list_store_append (store, &iter);
        
        // 0=ROOT, 1=ERROR
        if (i == 0) pic = projectgui_get_status_pixbuf (0);
        if (!g_file_test (tmp, G_FILE_TEST_EXISTS)) {
                    pic = projectgui_get_status_pixbuf (1);
                }

        gtk_list_store_set (store, &iter, 0, pic, 1, name, 2, path, 3, tmp, -1);
    }
    return amount;
}

GdkPixbuf* projectgui_get_status_pixbuf (int status) {
    GtkWidget* iv = GTK_WIDGET (gtk_invisible_new());

    switch (status) {
        case 0:
            return gtk_widget_render_icon (iv, GTK_STOCK_HOME,
                                               GTK_ICON_SIZE_MENU,
                                               NULL);
        case 1:
            return gtk_widget_render_icon (iv, GTK_STOCK_STOP,
                                               GTK_ICON_SIZE_MENU,
                                               NULL);
        default:
            return NULL;
    }
}

void projectgui_enable (GuProject* pr, GuProjectGui* prgui) {
    const gchar* projbasename = g_path_get_basename (pr->projfile);
    const gchar* projrootpath = g_path_get_dirname (pr->rootfile);
    
    gtk_label_set_text (prgui->proj_name, projbasename);
    gtk_label_set_text (prgui->proj_path, projrootpath);
    gtk_label_set_text (prgui->proj_nroffiles, 
                        g_strdup_printf("%d", pr->nroffiles));
    
    // for visible information when window is shrinked, see #439 -A
    gtk_widget_set_tooltip_text 
                        (GTK_WIDGET (prgui->proj_name), projbasename);
    gtk_widget_set_tooltip_text 
                        (GTK_WIDGET (prgui->proj_path), projrootpath);
                        
    gtk_widget_set_sensitive (GTK_WIDGET (prgui->proj_addbutton), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (prgui->proj_rembutton), TRUE);
    tablabel_set_bold_text (g_active_tab->page);
}

void projectgui_disable (GuProject* pr, GuProjectGui* prgui) {
    
    gtk_list_store_clear (gui->projectgui->list_projfiles);
    
    gtk_label_set_text (prgui->proj_name, "");
    gtk_label_set_text (prgui->proj_path, "");
    gtk_label_set_text (prgui->proj_nroffiles, "");

    gtk_widget_set_tooltip_text 
                        (GTK_WIDGET (prgui->proj_name), "");
    gtk_widget_set_tooltip_text 
                        (GTK_WIDGET (prgui->proj_path), "");
    
    gtk_widget_set_sensitive (GTK_WIDGET (prgui->proj_addbutton), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (prgui->proj_rembutton), FALSE);
}

G_MODULE_EXPORT
void on_projfile_add_clicked (GtkWidget* widget, void* user) {
    gchar* selected = NULL;
    
    selected = get_open_filename (TYPE_LATEX);
    
    if (selected) {
        if (project_add_document (gummi->project->projfile, selected)) {
            int amount = projectgui_list_projfiles (gummi->project->projfile);
            gtk_label_set_text (gui->projectgui->proj_nroffiles, 
                                g_strdup_printf("%d", amount));
            gui_open_file (selected);
        }
        else {
            statusbar_set_message ("Error adding document to the project..");
        }
    }
    g_free (selected);
}

G_MODULE_EXPORT
void on_projfile_rem_clicked (GtkWidget* widget, void* user) {
    GtkTreeIter iter;
    gchar* value;
    
    GtkTreeModel* model = GTK_TREE_MODEL (gui->projectgui->list_projfiles);
    GtkTreeSelection* selection = gtk_tree_view_get_selection 
                                         (gui->projectgui->proj_treeview);
                                         
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, 3, &value, -1);
    
        if (project_remove_document (gummi->project->projfile, value)) {
            int amount = projectgui_list_projfiles (gummi->project->projfile);
            gtk_label_set_text (gui->projectgui->proj_nroffiles, 
                                g_strdup_printf("%d", amount));
        }
    }
}
