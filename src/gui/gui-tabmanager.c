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

    tm->tabs = NULL;
    tm->active_editor = NULL;
    tm->active_page = NULL;
    return tm;
}

gboolean tabmanager_tab_pop_active (GuTabmanagerGui* tm) {
    gint position = gtk_notebook_get_current_page(tm->notebook);
    gint total = gtk_notebook_get_n_pages(tm->notebook);

    if (total == 0) return FALSE;
    GuTabContext* tab = g_list_nth(tm->tabs, position)->data;

    tm->tabs = g_list_remove(tm->tabs, tab);
    tabmanager_set_active_tab(tm, total -2);
    editor_destroy(tab->editor);
    gtk_notebook_remove_page(tm->notebook, position);
    g_free(tab);

    return (total != 1);
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

void tabmanager_change_label (GuTabmanagerGui* tm, const gchar *filename) {
    GtkWidget *tablabel;
    GtkWidget *page;
    
    gint cur = gtk_notebook_get_current_page(tm->notebook);
    page = gtk_notebook_get_nth_page(tm->notebook, cur);
    
    /* gtk_notebook_set_tab_label_text ()
     * could use that, but might not work for more complicated
     * label (with add/close button) */
    tablabel = tabmanager_create_label(tm, filename);
    gtk_notebook_set_tab_label (tm->notebook, page, tablabel);
}

void tabmanager_set_active_tab(GuTabmanagerGui* tm, gint position) {
    if (position == -1) {
        tm->active_tab = NULL;
        tm->active_editor = NULL;
        tm->active_page = NULL;
    } else {
        tm->active_tab =
            GU_TAB_CONTEXT(g_list_nth_data(tm->tabs, position));
        tm->active_editor =
            GU_TAB_CONTEXT(g_list_nth_data(tm->tabs, position))->editor;
        tm->active_page =
            GU_TAB_CONTEXT(g_list_nth_data(tm->tabs, position))->page;
    }
}

GuTabContext* tabmanager_create_tab(GuTabmanagerGui* tm, GuEditor* ec,
                                    const gchar* filename) {
    GuTabContext* tab = g_new0(GuTabContext, 1);

    tab->editor = ec;
    tab->page = gtk_scrolled_window_new (NULL, NULL);
    tab->label = tabmanager_create_label(tm, filename);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(tab->page),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    return tab;
}

gint tabmanager_tab_replace_active(GuTabmanagerGui* tm, GuEditor* ec,
                                   const gchar* filename) {
    tm->active_tab->editor = ec;
    tabmanager_change_label(tm, filename);
    editor_destroy(tm->active_editor);
    gtk_container_remove (GTK_CONTAINER (tm->active_tab->page),
                          GTK_WIDGET (tm->active_editor->view));
    gtk_container_add (GTK_CONTAINER (tm->active_tab->page),
                       GTK_WIDGET (ec->view));
    gtk_widget_show(GTK_WIDGET(ec->view));
    return gtk_notebook_page_num(tm->notebook, tm->active_page);
}

gint tabmanager_tab_push(GuTabmanagerGui* tm, GuTabContext* tc) {
    gint pos = 0;

    tm->tabs = g_list_append(tm->tabs, tc);
    gtk_container_add (GTK_CONTAINER (tc->page),
                       GTK_WIDGET(tc->editor->view));
    pos = gtk_notebook_append_page (GTK_NOTEBOOK (tm->notebook), tc->page,
                                    tc->label);

    gtk_widget_grab_focus(GTK_WIDGET(tc->editor->view));

    gtk_widget_show(tc->page);
    gtk_widget_show(GTK_WIDGET(tc->editor->view));

    return pos;
}

void tabmanager_switch_tab(GuTabmanagerGui* tm, gint pos) {
    gtk_notebook_set_current_page(tm->notebook, pos);
}

gint tabmanager_get_editor_position(GuTabmanagerGui* tm, GuEditor* ec) {
    int i = 0;
    for (i = 0; i < g_list_length(tm->tabs); ++i) {
        if (GU_TAB_CONTEXT(g_list_nth(tm->tabs, i)->data)->editor == ec)
            return i;
    }
    return -1;
}
