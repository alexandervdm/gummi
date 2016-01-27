/**
 * @file   gui-project.h
 * @brief
 *
 * Copyright (C) 2009-2016 Gummi Developers
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

#ifndef __GUMMI_GUI_PROJECT_H__
#define __GUMMI_GUI_PROJECT_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "project.h"

#define g_menugui gui->menugui

#define GU_PROJECT_GUI(x) ((GuProjectGui*)x)
typedef struct _GuProjectGui GuProjectGui;

struct _GuProjectGui {
    GtkListStore* list_projfiles;
    GtkTreeView* proj_treeview;

    GtkButton* proj_addbutton;
    GtkButton* proj_rembutton;

    GtkLabel* proj_name;
    GtkLabel* proj_path;
    GtkLabel* proj_nroffiles;

};

GuProjectGui* projectgui_init (GtkBuilder* builder);

void projectgui_enable (GuProject* pr, GuProjectGui* prgui);
void projectgui_disable (GuProject* pr, GuProjectGui* prgui);

GdkPixbuf* projectgui_get_status_pixbuf (int status);

void projectgui_set_rootfile (gint position);

int projectgui_list_projfiles (gchar* active_proj);

#endif /* __GUMMI_GUI_PROJECT_H__ */
