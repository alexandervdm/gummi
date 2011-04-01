/**
 * @file    tabmanager.h
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

#ifndef GUMMI_TABMANAGER_H
#define GUMMI_TABMANAGER_H

#include <gtk/gtk.h>

#include "editor.h"

/* TODO: is this a poor practice? it sure is clean/handy..
 * Yes, we usually avoid defining macros that relies on local magic variable
 * names */
#define g_active_editor gummi->tabmanager->active_editor

typedef struct _GuTabmanager GuTabmanager;

struct _GuTabmanager {
    GList *editors;
    GList *pages;
    GuEditor* active_editor;
    GtkNotebook* active_page;
};


GuTabmanager* tabmanager_init (GuEditor* first);
void tabmanager_create_tab(GuTabmanager* tc, GuEditor *new_editor,
                           const gchar* filename);

/*
 * @brief remove a tab
 * @return TURE means the removed tab is last one
 */
gboolean tabmanager_remove_tab(GuTabmanager* tc, gint pagenr);
void tabmanager_set_active_tab(GuTabmanager* tc, gint pagenr);
gint tabmanager_get_position_from_editor(GuTabmanager* tc, GuEditor* ec);
gint tabmanager_get_position_from_page(GuTabmanager* tc, GtkWidget* page);

#endif /* __GUMMI_TABMANAGER_H__ */

