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

#define GU_TAB_CONTEXT(x) ((GuTabContext*)x)
typedef struct _GuTabContext GuTabContext;

struct _GuTabContext {
    GuEditor* editor;
    GtkWidget* page;
    GtkWidget* label;
};

#define GU_TABMANAGER_GUI(x) ((GuTabmanagerGui*)x)
typedef struct _GuTabmanagerGui GuTabmanagerGui;

struct _GuTabmanagerGui {
    GtkNotebook *notebook;

    GuTabContext* active_tab;
    GuEditor* active_editor;
    GtkWidget* active_page;

    GList* tabs;
};

GuTabmanagerGui* tabmanagergui_init (GtkBuilder* builder);

GuTabContext* tabmanager_create_tab(GuTabmanagerGui* tm, GuEditor* ec,
                                    const gchar* filename);
gint tabmanager_tab_replace_active(GuTabmanagerGui* tm, GuEditor* ec,
                                   const gchar* filename);

gint tabmanager_tab_push(GuTabmanagerGui* tm, GuTabContext* tc);
gboolean tabmanager_tab_pop_active (GuTabmanagerGui* tm);
void tabmanager_switch_tab(GuTabmanagerGui* tm, gint pos);

GtkWidget* tabmanager_create_label (GuTabmanagerGui* tm, const gchar *filename);
void tabmanager_change_label (GuTabmanagerGui* tm, const gchar *filename);

void tabmanager_set_active_tab(GuTabmanagerGui* tm, gint position);
gint tabmanager_create_unsavednr (GuTabmanagerGui* tm);

gint tabmanager_get_editor_position(GuTabmanagerGui* tm, GuEditor* ec);

#endif /* __GUMMI_GUI_TABMANAGER_H__ */
