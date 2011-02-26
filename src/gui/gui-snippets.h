/**
 * @file   gui-snippet.h
 * @brief  Handle snippets and provide edit/new/delete function
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

#ifndef __GUMMI_GUI_SNIPPETS__
#define __GUMMI_GUI_SNIPPETS__

#include <glib.h>
#include <gtk/gtk.h>

#include "snippets.h"

typedef struct _GuSnippetsGui {
    GtkWidget* snippetswindow;
    GtkTreeView* snippets_tree_view;
    GtkScrolledWindow* snippet_scroll;
    GtkEntry* tab_trigger_entry;
    GtkEntry* accelerator_entry;
    GtkListStore* list_snippets;
    GtkCellRendererText* snippet_renderer;
    GtkSourceView* sourceview;
    GtkSourceBuffer* sourcebuffer;
    slist* current;
} GuSnippetsGui;

GuSnippetsGui* snippetsgui_init(GtkWidget* mainwindow);
void snippetsgui_main(GuSnippetsGui* snip);
void snippetsgui_update_snippet(GuSnippets* sc);
void on_button_new_snippet_clicked(GtkBuilder* widget, void* user);
void on_button_remove_snippet_clicked(GtkBuilder* widget, void* user);
void on_tab_trigger_entry_changed(GtkEntry* entry, void* user);
void on_accelerator_entry_focus_in_event(GtkWidget* widget, void* user);
void on_accelerator_entry_focus_out_event(GtkWidget* widget, void* user);
gboolean on_accelerator_entry_key_press_event(GtkWidget* widget,
        GdkEventKey* event, void* user);
void on_snippetsgui_close_clicked(GtkWidget* widget, void* user);
void on_snippets_tree_view_row_activated(GtkTreeView* view, void* user);
void on_snippet_renderer_edited(GtkCellRendererText* renderer, gchar *path,
        gchar* name, void* user);
gboolean on_snippet_source_buffer_key_release(GtkWidget* widget, void* user);

#endif /* __GUMMI_GUI_SNIPPETS__ */
