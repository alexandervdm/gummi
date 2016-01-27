/**
 * @file   gui-search.h
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

#ifndef __GUMMI_GUI_SEARCH_H__
#define __GUMMI_GUI_SEARCH_H__

#include <glib.h>
#include <gtk/gtk.h>

#define GU_SEARCH_GUI(x) ((GuSearchGui*)x)
typedef struct _GuSearchGui GuSearchGui;

struct _GuSearchGui {
    GtkWidget* searchwindow;
    GtkEntry* searchentry;
    GtkEntry* replaceentry;
    gboolean backwards;
    gboolean matchcase;
    gboolean wholeword;
    gchar* prev_search;
    gchar* prev_replace;
};

GuSearchGui* searchgui_init (GtkBuilder* builder);
void searchgui_main (GuSearchGui* gc);
void on_toggle_matchcase_toggled (GtkWidget* widget, void* user);
void on_toggle_wholeword_toggled (GtkWidget* widget, void* user);
void on_toggle_backwards_toggled (GtkWidget* widget, void* user);
void on_searchgui_text_changed (GtkEditable* editable, void* user);
gboolean on_button_searchwindow_close_clicked (GtkWidget* widget, void* user);
void on_button_searchwindow_find_clicked (GtkWidget* widget, void* user);
void on_button_searchwindow_replace_next_clicked (GtkWidget* widget, void* user);
void on_button_searchwindow_replace_all_clicked (GtkWidget* widget, void* user);

#endif /* __GUMMI_GUI_SEARCH_H__ */
