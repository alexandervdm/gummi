/**
 * @file    gui-project.c
 * @brief
 *
 * Copyright (C) 2010-2011 Gummi-Dev Team <alexvandermey@gmail.com>
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

#include "gui-main.h"
#include "gui-tabmanager.h"
#include "project.h"

extern GummiGui* gui;

void projectgui_set_rootfile (gint position) {
    tabmanagergui_switch_tab (gui->tabmanagergui, position);
    tablabel_set_bold_text (gui->tabmanagergui, position);
}

void projectgui_list_projopened (GtkComboBox* combo, GtkListStore* store) {
    GtkTreeIter iter;
    gint i, tabnr;
    GuEditor *ec;
    
    gtk_list_store_clear (store);
    GList* tabs = tabmanagergui_get_all_tabs(gui->tabmanagergui);
    tabnr = g_list_length(tabs);

    for (i=0; i < tabnr; i++) {
        ec = GU_TAB_CONTEXT (g_list_nth_data (tabs, i))->editor;
        if (ec->projfile != NULL) {
            gchar* basename = g_path_get_basename (ec->projfile);
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter, 0, basename, 1, ec->projfile, -1);
        }
    }
    gtk_combo_box_set_active (combo, 0);
}

void projectgui_list_projfiles (GtkListStore* store, gchar* active_proj) {
    gchar* content = NULL;
    GtkTreeIter iter;
    GError* err = NULL;
    GList* files = NULL;
    gint amount, i;
    gchar* name;
    gchar* path;
    
    gtk_list_store_clear (store);

    if (!g_file_get_contents (active_proj, &content, NULL, &err)) {
        slog (L_ERROR, "%s\n", err->message);
        return;
    }
    
    files = project_list_files (content);
    amount = g_list_length (files);
    
    for (i=0; i < amount; i++) {
        gchar* tmp = g_list_nth_data (files, i);
        name = g_path_get_basename (tmp);
        path = g_path_get_dirname (tmp);
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, name, 1, path, -1);
    }
}





