/**
 * @file    tabmanager.c
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

#include "tabmanager.h"

#include <assert.h>

#include "environment.h"
#include "gui/gui-main.h"

/* IMHO, tabmanager class can be merged with gui-editortabs since these classes
 * mainly process GUI related actions. For example, tabmanager::pages stores
 * GtkNotebook pages and thus is GUI related. */

/* XXX: remove this after the class is merged !!! seriously. */
extern GummiGui* gui;

GuTabmanager* tabmanager_init (GuEditor *first) {
    GuTabmanager* tm = g_new0 (GuTabmanager, 1);
    tm->editors = NULL;
    tm->pages = NULL;
    tm->active_editor = first;
    tm->active_page = NULL;
    return tm;
}

void tabmanager_create_tab(GuTabmanager* tc, GuEditor *editor,
                           const gchar* filename) {
    GtkWidget *new_page;
    
    /* editor */
    assert(editor != NULL);
    tc->editors = g_list_append(tc->editors, editor);
    gummi_new_environment(tc->editors, editor, filename);
    
    /* page */
    gint pagenr = editortabsgui_create_page(editor, filename);
    new_page = gtk_notebook_get_nth_page(g_tabs_notebook, pagenr);
    tc->pages = g_list_append(tc->pages, new_page);
    
    //tabmanager_set_active_tab(pagenr);

    gtk_notebook_set_current_page(g_tabs_notebook, pagenr);
    gtk_widget_grab_focus(GTK_WIDGET(editor->view));
}

gboolean tabmanager_remove_tab(GuTabmanager* tc, gint pagenr) {
    GtkWidget *current_page;

    /*TODO: remove direct gtk calls from this non-gui class */
    current_page = gtk_notebook_get_nth_page(g_tabs_notebook, pagenr);
    gint indexnr = tabmanager_get_position_from_page(tc, current_page);
    
    tc->editors = g_list_remove (tc->editors,
                                 g_list_nth_data(tc->editors, indexnr));
    tc->pages = g_list_remove (tc->pages, g_list_nth_data(tc->pages, indexnr));
    editortabsgui_remove_page();

    if (!g_list_length(tc->editors))
        return TRUE;
    return FALSE;
}

void tabmanager_set_active_tab(GuTabmanager* tc, gint position) {
    tc->active_editor = g_list_nth_data(tc->editors, position);
    tc->active_page = g_list_nth_data(tc->pages, position);
}

gint tabmanager_get_position_from_editor(GuTabmanager* tc, GuEditor* ec) {
    return g_list_index(tc->editors, ec);
}

gint tabmanager_get_position_from_page(GuTabmanager* tc, GtkWidget* page) {
    return g_list_index(tc->pages, page);
}
