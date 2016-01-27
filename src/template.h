/**
 * @file    template.h
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

#ifndef __GUMMI_TEMPLATE_H__
#define __GUMMI_TEMPLATE_H__

#include <glib.h>
#include <gtk/gtk.h>

typedef struct _templdata {
    gchar* itemname;
    gchar* itemdata;
} templdata;

typedef struct _Template {
    GtkWindow* templatewindow;
    GtkTreeView* templateview;
    GtkListStore* list_templates;
    GtkCellRendererText* template_render;
    GtkTreeViewColumn* template_col;
    GtkLabel* template_label;
    GtkWidget* template_add;
    GtkWidget* template_remove;
    GtkWidget* template_open;
} GuTemplate;


GuTemplate* template_init (GtkBuilder* builder);
void template_setup ();
void template_add_new_entry (GuTemplate* t);
void template_remove_entry (GuTemplate* t);
void template_create_file (GuTemplate* t, gchar* filename, gchar* text);
gchar* template_get_selected_path (GuTemplate* t);
void template_data_free(templdata* data);

#endif /* __GUMMI_TEMPLATE_H__ */
