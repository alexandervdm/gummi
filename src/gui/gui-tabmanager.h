/**
 * @file   gui-editortabs.h
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

#ifndef __GUMMI_GUI_TABMANAGER_H__
#define __GUMMI_GUI_TABMANAGER_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "editor.h"


#define GU_TABMANAGER_GUI(x) ((GuTabmanagerGui*)x)
typedef struct _GuTabmanagerGui GuTabmanagerGui;

struct _GuTabmanagerGui {
    GtkNotebook *notebook;

    GuEditor* active_editor;
    GtkNotebook* active_page;

    GList *editors;
    GList *pages;
};

GuTabmanagerGui* tabmanagergui_init (GtkBuilder* builder);

gint tabmanager_create_page (GuTabmanagerGui* tm, GuEditor* editor,
                             const gchar* filename);
void tabmanager_remove_page (GuTabmanagerGui* tm);                             
                             
GtkWidget* tabmanager_create_label (GuTabmanagerGui* tm, const gchar *filename);
void tabmanager_change_label (GuTabmanagerGui* tc, const gchar *filename);

void tabmanager_set_active_tab(GuTabmanagerGui* tc, gint position);

gint tabmanager_create_unsavednr (GuTabmanagerGui* tc);

gint tabmanager_push_editor(GuTabmanagerGui* tc, GuEditor* ec);
gint tabmanager_push_page(GuTabmanagerGui* tc, GtkWidget* pg);
gint tabmanager_get_editor_position(GuTabmanagerGui* tc, GuEditor* ec);
gint tabmanager_get_page_position(GuTabmanagerGui* tc, GtkWidget* pg);

#endif /* __GUMMI_GUI_TABMANAGER_H__ */
