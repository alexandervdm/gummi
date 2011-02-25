/**
 * @file   gui-snippets.h
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

#include "gui/gui-snippets.h"

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourceiter.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestyleschememanager.h>
#include <gtksourceview/gtksourceview.h>

#include "environment.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuSnippetsGui* snippetsgui_init(GtkWidget* mainwindow) {
    GuSnippetsGui* s = g_new0(GuSnippetsGui, 1);
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, DATADIR"/snippets.glade", NULL);
    gtk_builder_set_translation_domain(builder, PACKAGE);

    s->snippetswindow =
        GTK_WIDGET(gtk_builder_get_object(builder, "snippetswindow"));
    s->snippets_tree_view =
        GTK_TREE_VIEW(gtk_builder_get_object(builder, "snippets_tree_view"));
    s->snippet_scroll =
        GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "snippet_scroll"));
    s->tab_trigger_entry = 
        GTK_ENTRY(gtk_builder_get_object(builder, "tab_trigger_entry"));
    s->accelerator_entry = 
        GTK_ENTRY(gtk_builder_get_object(builder, "accelerator_entry"));
    s->list_snippets = 
        GTK_LIST_STORE(gtk_builder_get_object(builder, "list_snippets"));
    GtkSourceLanguageManager* manager = gtk_source_language_manager_new();
    GtkSourceLanguage* lang = gtk_source_language_manager_get_language(manager,
            "latex");
    s->sourcebuffer = gtk_source_buffer_new_with_language(lang);
    s->sourceview =
        GTK_SOURCE_VIEW(gtk_source_view_new_with_buffer(s->sourcebuffer));
    gtk_container_add(GTK_CONTAINER(s->snippet_scroll),
            GTK_WIDGET(s->sourceview));

    /* set style scheme */
    slist* current = gummi->snippets->head;
    GtkTreeIter iter;

    while (current) {
        if (current->first[0] != '#') {
            gtk_list_store_append(s->list_snippets, &iter);
            gtk_list_store_set(s->list_snippets, &iter, 0, current->first, -1);
        }
        current = current->next;
    }

    g_signal_connect(s->sourceview, "key-release-event",
            G_CALLBACK(on_snippet_source_buffer_key_release), NULL);

    gtk_window_set_transient_for(GTK_WINDOW(s->snippetswindow), 
            GTK_WINDOW(mainwindow));
    gtk_builder_connect_signals(builder, NULL);

    return s;
}

void snippetsgui_main(GuSnippetsGui* snip) {
    gtk_widget_show_all(snip->snippetswindow);
}

void on_snippetsgui_close_clicked(GtkWidget* widget, void* user) {
    gtk_widget_hide_all(gui->snippetsgui->snippetswindow);
    snippets_save(gummi->snippets);
}

void on_button_new_snippet_clicked(GtkBuilder* widget, void* user) {
    
}

void on_button_remove_snippet_clicked(GtkBuilder* widget, void* user) {
    
}

void on_tab_trigger_entry_changed(GtkEntry* entry, void* user) {
    gchar* key = NULL;
    gchar* accel = NULL;
    gchar* new_key = gtk_entry_get_text(entry);
    slist* target = gui->snippetsgui->current;
    key = g_strndup(target->first, strstr(target->first, ",") - target->first);
    accel = g_strdup(strstr(target->first, ",") + 1);
    g_free(target->first);
    target->first = g_strdup_printf("%s,%s", new_key, accel);
}

void on_accelerator_entry_focus_in_event(GtkWidget* widget, void* user) {
    
}

void on_accelerator_entry_focus_out_event(GtkWidget* widget, void* user) {
    
}

void on_snippets_tree_view_cursor_changed(GtkTreeView* view, void* user) {
    if (!gui) return;

    GuSnippetsGui* s = gui->snippetsgui;
    GtkTreeIter iter;
    gchar* key_accel = NULL;
    gchar* key = NULL;
    gchar* accel = NULL;
    gchar* snippet = NULL;
    GtkTreeModel* model = GTK_TREE_MODEL(gtk_tree_view_get_model(view));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(view);

    gtk_tree_selection_get_selected(selection, &model, &iter);
    gtk_tree_model_get(model, &iter, 0, &key_accel, -1);

    /* Record current activated snippet */
    s->current = slist_find_index_of(gummi->snippets->head,
            key_accel, FALSE, FALSE);

    key = g_strndup(key_accel, strstr(key_accel, ",") - key_accel);
    accel = g_strdup(strstr(key_accel, ",") + 1);
    snippet = snippets_get_value(gummi->snippets, key);

    /* Set text buffer */
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(s->sourcebuffer), snippet, -1);
    /* Set entries */
    gtk_entry_set_text(s->tab_trigger_entry, key);
    gtk_entry_set_text(s->accelerator_entry, accel);

    g_free(key_accel);
    g_free(accel);
    g_free(key);
}

gboolean on_snippet_source_buffer_key_release(GtkWidget* widget, void* user) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(gui->snippetsgui->sourcebuffer),
                &start, &end);
    gchar* text = gtk_text_iter_get_text(&start, &end);
    g_free(gui->snippetsgui->current->second);
    gui->snippetsgui->current->second = g_strdup(text);
    g_free(text);
    return FALSE;
}
