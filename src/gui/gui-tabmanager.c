/**
 * @file   gui-tabmanager.c
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

#include "gui-tabmanager.h"

#include "gui-main.h"
#include "environment.h"


GuTabmanagerGui* tabmanagerguigui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuTabmanagerGui* tm = g_new0 (GuTabmanagerGui, 1);

    tm->notebook =
        GTK_NOTEBOOK (gtk_builder_get_object (builder, "tab_notebook"));

    g_object_set (tm->notebook, "tab-border", 0, NULL);

    tm->tabs = NULL;
    tm->active_editor = NULL;
    tm->active_page = NULL;
    return tm;
}

GuTabLabel* tablabel_new (GuTabContext* tab, const gchar* filename) {
    static unsigned count = 0;
    GtkRcStyle* rcstyle = NULL;
    GtkWidget* image = NULL;
    GuTabLabel* tl = g_new0(GuTabLabel, 1);

    tl->unsave = ++count;
    tl->ebox = gtk_event_box_new ();
    tl->hbox = GTK_HBOX (gtk_hbox_new (FALSE, 0));

    gtk_event_box_set_visible_window (GTK_EVENT_BOX (tl->ebox), FALSE);
    gtk_container_add (GTK_CONTAINER(tl->ebox), GTK_WIDGET (tl->hbox));

    tl->label = GTK_LABEL (gtk_label_new (filename));
    tablabel_update_label_text (tl, filename, FALSE);

    gtk_box_pack_start (GTK_BOX (tl->hbox), GTK_WIDGET (tl->label), TRUE,
                        TRUE, 5);

    tl->close = GTK_BUTTON (gtk_button_new());
    image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
    gtk_button_set_image (tl->close, image);
    g_object_set (tl->close, "relief", GTK_RELIEF_NONE,
                             "focus-on-click", FALSE, NULL);
    gtk_box_pack_start (GTK_BOX (tl->hbox), GTK_WIDGET (tl->close), FALSE,
                        FALSE, 0);
    g_signal_connect (tl->close,
                      "clicked",
                      G_CALLBACK (on_menu_close_activate),
                      tab);

    /* make it as small as possible */
    rcstyle = gtk_rc_style_new ();
    rcstyle->xthickness = rcstyle->ythickness = 0;
    gtk_widget_modify_style (GTK_WIDGET (tl->close), rcstyle);
    g_object_unref (rcstyle);

    gtk_widget_show_all (GTK_WIDGET (tl->hbox));

    return tl;
}

void tablabel_update_label_text (GuTabLabel* tl, const gchar* filename,
                                 gboolean modified) {
                       
    gchar* labeltext = NULL; 
    gchar* labelname = NULL;
    labelname = (filename)? g_path_get_basename (filename):
                        g_strdup_printf(_("Unsaved Document %d"), tl->unsave);
    labeltext = g_strdup_printf ("%s%s", (modified? "*": ""), labelname);
    gtk_label_set_text (tl->label, labeltext);
    g_free (labelname);
    g_free (labeltext);
}

gboolean tabmanagergui_tab_pop (GuTabmanagerGui* tm, GuTabContext* tab) {
    gint position = g_list_index (tm->tabs, tab);
    gint total = gtk_notebook_get_n_pages (tm->notebook);

    if (total == 0) return FALSE;

    tm->tabs = g_list_remove (tm->tabs, tab);
    tabmanagergui_set_active_tab (tm, total -2);
    editor_destroy (tab->editor);
    gtk_notebook_remove_page (tm->notebook, position);
    g_free (tab);

    return (total != 1);
}

void tabmanagergui_set_active_tab(GuTabmanagerGui* tm, gint position) {
    
    if (position == -1) {
        tm->active_tab = NULL;
        tm->active_editor = NULL;
        tm->active_page = NULL;
    } else {
        tm->active_tab =
            GU_TAB_CONTEXT (g_list_nth_data (tm->tabs, position));
        tm->active_editor =
            GU_TAB_CONTEXT (g_list_nth_data (tm->tabs, position))->editor;
        tm->active_page =
            GU_TAB_CONTEXT (g_list_nth_data (tm->tabs, position))->page;
    }
}

GuTabContext* tabmanagergui_create_tab(GuTabmanagerGui* tm, GuEditor* ec,
                                       const gchar* filename) {
    GuTabContext* tab = g_new0(GuTabContext, 1);

    tab->editor = ec;
    tab->page = gtk_scrolled_window_new (NULL, NULL);
    tab->tablabel = tablabel_new (tab, filename);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(tab->page),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    return tab;
}

gint tabmanagergui_tab_replace_active(GuTabmanagerGui* tm, GuEditor* ec,
                                      const gchar* filename) {
                                          
    tm->active_tab->editor = ec;
    editor_destroy(tm->active_editor);
    gtk_container_remove (GTK_CONTAINER (tm->active_tab->page),
                          GTK_WIDGET (tm->active_editor->view));
    gtk_container_add (GTK_CONTAINER (tm->active_tab->page),
                       GTK_WIDGET (ec->view));
    gtk_widget_show(GTK_WIDGET(ec->view));
    return gtk_notebook_page_num(tm->notebook, tm->active_page);
}

gint tabmanagergui_tab_push(GuTabmanagerGui* tm, GuTabContext* tc) {
    gint pos = 0;

    tm->tabs = g_list_append(tm->tabs, tc);
    gtk_container_add (GTK_CONTAINER (tc->page),
                       GTK_WIDGET(tc->editor->view));
    pos = gtk_notebook_append_page (GTK_NOTEBOOK (tm->notebook), tc->page,
                                    GTK_WIDGET (tc->tablabel->ebox));

    gtk_widget_show(tc->page);
    gtk_widget_show(GTK_WIDGET(tc->editor->view));
    gtk_widget_grab_focus(GTK_WIDGET(tc->editor->view));

    return pos;
}

void tabmanagergui_switch_tab(GuTabmanagerGui* tm, gint pos) {
    gtk_notebook_set_current_page(tm->notebook, pos);
}



GList* tabmanagergui_return_tablabels(GuTabmanagerGui* tm) {
    GList *labelnames = NULL;
    GuTabContext *tab = NULL;
    const gchar *text;
    guint items, i;

    items = g_list_length (tm->tabs);
    
    for (i = 0; i < items; i++) {
        
        tab = g_list_nth_data (tm->tabs, i);
        text = gtk_label_get_text (tab->tablabel->label);
        labelnames = g_list_append (labelnames, (gpointer)text);
    }
    return labelnames;
}

gboolean tabmanagergui_existing_tabs (GuTabmanagerGui* tm) {
    if (g_list_length(tm->tabs) != 0) {
        return TRUE;
    }
    return FALSE;
}

