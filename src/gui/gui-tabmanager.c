/**
 * @file   gui-tabmanager.c
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

#include "gui-tabmanager.h"

GuTabmanagerGui* tabmanagergui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuTabmanagerGui* tm = g_new0 (GuTabmanagerGui, 1);

    tm->notebook =
        GTK_NOTEBOOK (gtk_builder_get_object (builder, "tab_notebook"));

    tm->editors = NULL;
    tm->pages = NULL;
    tm->active_editor = NULL;
    tm->active_page = NULL;
    return tm;
}

gint tabmanager_create_page (GuTabmanagerGui* tm, GuEditor* editor,
                             const gchar* filename) {
    GtkWidget *scrollwindow;
    GtkWidget *tablabel;
    GtkWidget *page;

    /* creating tab object; scrollwindow */
    scrollwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwindow),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add (GTK_CONTAINER (scrollwindow), GTK_WIDGET(editor->view));

    /* creating tab object; label */
    tablabel = tabmanager_create_label(tm, filename);

    /* adding new tab to the gui */
    gint position = gtk_notebook_append_page (
            GTK_NOTEBOOK (tm->notebook), scrollwindow, tablabel);

    page = gtk_notebook_get_nth_page(tm->notebook, position);
    tabmanager_push_page(tm, page);

    gtk_widget_show(scrollwindow);
    gtk_widget_show(GTK_WIDGET(editor->view));

    gtk_notebook_set_current_page(tm->notebook, position);
    gtk_widget_grab_focus(GTK_WIDGET(editor->view));
    return position;
}

void tabmanager_remove_page (GuTabmanagerGui* tm) {
	gint position = gtk_notebook_get_current_page(tm->notebook);
    gtk_notebook_remove_page(tm->notebook, position);
    tm->editors = g_list_remove(tm->editors, tm->active_editor);
    tm->pages = g_list_remove(tm->pages, tm->active_page);
}

GtkWidget* tabmanager_create_label (GuTabmanagerGui* tm, const gchar *filename) {
    GtkWidget *tablabel;
    gchar *tabname;

    gint nr_pages = gtk_notebook_get_n_pages (tm->notebook);

    if (!filename)
        tabname = g_strdup_printf ("Unsaved Document %d", (nr_pages+1));
    else 
        tabname = g_path_get_basename (filename);

    tablabel = gtk_label_new (tabname);
    gtk_widget_set_tooltip_text (tablabel, filename);

    g_free(tabname);

    return tablabel;
}

void tabmanager_change_label (GuTabmanagerGui* tc, const gchar *filename) {
    GtkWidget *tablabel;
    GtkWidget *page;
    
    gint cur = gtk_notebook_get_current_page(tc->notebook);
    page = gtk_notebook_get_nth_page(tc->notebook, cur);
    
    /* gtk_notebook_set_tab_label_text ()
     * could use that, but might not work for more complicated
     * label (with add/close button) */
    tablabel = tabmanager_create_label(tc, filename);
    gtk_notebook_set_tab_label (tc->notebook, page, tablabel);
}

void tabmanager_set_active_tab(GuTabmanagerGui* tc, gint position) {
    tc->active_editor = g_list_nth_data(tc->editors, position);
    tc->active_page = g_list_nth_data(tc->pages, position);
}

gint tabmanager_push_editor(GuTabmanagerGui* tc, GuEditor* ec) {
    tc->editors = g_list_append(tc->editors, ec);
    gint position = g_list_index(tc->editors, ec);
    return position;
}

gint tabmanager_push_page(GuTabmanagerGui* tc, GtkWidget* pg) {
    tc->pages = g_list_append(tc->pages, pg);
    gint position = g_list_index(tc->pages, pg);
    return position;
}

gint tabmanager_get_editor_position(GuTabmanagerGui* tc, GuEditor* ec) {
    return g_list_index(tc->editors, ec);
}

gint tabmanager_get_page_position(GuTabmanagerGui* tc, GtkWidget* pg) {
    return g_list_index(tc->pages, pg);
}
