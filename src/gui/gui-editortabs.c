/**
 * @file   gui-editortabs.c
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

#include "gui-editortabs.h"

#include <gtk/gtk.h>

#include "environment.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuEditortabsGui* editortabsgui_init(GtkBuilder* builder) {
    g_return_val_if_fail(GTK_IS_BUILDER(builder), NULL);
    
    GuEditortabsGui* et = g_new0(GuEditortabsGui, 1);
    
    et->tab_notebook = gtk_builder_get_object(builder, "tab_notebook");
    
    return et;
}

void editortabsgui_create_tab(GuEditor* editor, const gchar* filename) {
    GtkWidget *scrollwindow;
    GtkWidget *tablabel;
    gchar *tabname;
    
    //gummi_new_environment is central function to create new editors.

    gint nr_pages = gtk_notebook_get_n_pages(g_tabs_notebook);
    
    if (filename == NULL) 
        tabname = g_strdup_printf("Unsaved Document %d", (nr_pages+1));
    else 
        tabname = g_path_get_basename(filename);
    tablabel = gtk_label_new(tabname);
    
    scrollwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrollwindow),
                                    GTK_POLICY_AUTOMATIC, 
                                    GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(scrollwindow), GTK_WIDGET(editor->sourceview));
    
    gtk_notebook_append_page(
        GTK_NOTEBOOK(g_tabs_notebook), scrollwindow, tablabel);
}


GuEditor* get_active_editor(void) {
    return gummi->editor;
}

