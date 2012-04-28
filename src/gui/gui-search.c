/**
 * @file   gui-search.c
 * @brief
 *
 * Copyright (C) 2009-2012 Gummi-Dev Team <alexvandermey@gmail.com>
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

#include "gui-search.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "editor.h"
#include "environment.h"
#include "utils.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuSearchGui* searchgui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuSearchGui* s = g_new0 (GuSearchGui, 1);
    s->searchwindow =
        GTK_WIDGET (gtk_builder_get_object (builder, "searchwindow"));
    s->searchentry =
        GTK_ENTRY (gtk_builder_get_object (builder, "searchentry"));
    s->replaceentry =
        GTK_ENTRY (gtk_builder_get_object (builder, "replaceentry"));
    s->matchcase = FALSE;
    s->backwards = FALSE;
    s->wholeword = FALSE;
    s->prev_search = NULL;
    s->prev_replace = NULL;
    g_signal_connect (s->searchentry, "changed",
            G_CALLBACK (on_searchgui_text_changed), NULL);
    return s;
}

void searchgui_main (GuSearchGui* gc) {
    gtk_entry_set_text (gc->searchentry, gc->prev_search? gc->prev_search:"");
    gtk_entry_set_text (gc->replaceentry, gc->prev_replace? gc->prev_replace:"");
    gtk_widget_grab_focus (GTK_WIDGET (gc->searchentry));
    gtk_widget_show_all (GTK_WIDGET (gc->searchwindow));
    slog_set_gui_parent (GTK_WINDOW (gc->searchwindow));
}

G_MODULE_EXPORT
void on_toggle_matchcase_toggled (GtkWidget *widget, void* user) {
    gui->searchgui->matchcase =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    g_active_editor->replace_activated = FALSE;
}

G_MODULE_EXPORT
void on_toggle_wholeword_toggled (GtkWidget *widget, void* user) {
    gui->searchgui->wholeword =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    g_active_editor->replace_activated = FALSE;
}

G_MODULE_EXPORT
void on_toggle_backwards_toggled (GtkWidget *widget, void* user) {
    gui->searchgui->backwards =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    g_active_editor->replace_activated = FALSE;
}

void on_searchgui_text_changed (GtkEditable *editable, void* user) {
    g_active_editor->replace_activated = FALSE;
}

G_MODULE_EXPORT
gboolean on_button_searchwindow_close_clicked (GtkWidget* widget, void* user) {
    g_free (gui->searchgui->prev_search);
    g_free (gui->searchgui->prev_replace);
    gui->searchgui->prev_search = g_strdup (gtk_entry_get_text (
                gui->searchgui->searchentry));
    gui->searchgui->prev_replace = g_strdup (gtk_entry_get_text (
                gui->searchgui->replaceentry));
    gtk_widget_hide (GTK_WIDGET (gui->searchgui->searchwindow));
    slog_set_gui_parent (GTK_WINDOW (gui->mainwindow));
    return TRUE;
}

G_MODULE_EXPORT
void on_button_searchwindow_find_clicked (GtkWidget* widget, void* user) {
    editor_start_search (g_active_editor,
            gtk_entry_get_text (gui->searchgui->searchentry),
            gui->searchgui->backwards,
            gui->searchgui->wholeword,
            gui->searchgui->matchcase
            );
}

G_MODULE_EXPORT
void on_button_searchwindow_replace_next_clicked (GtkWidget* widget, void* user) {
    editor_start_replace_next (g_active_editor,
            gtk_entry_get_text (gui->searchgui->searchentry),
            gtk_entry_get_text (gui->searchgui->replaceentry),
            gui->searchgui->backwards,
            gui->searchgui->wholeword,
            gui->searchgui->matchcase
            );
}

G_MODULE_EXPORT
void on_button_searchwindow_replace_all_clicked (GtkWidget* widget, void* user) {
    editor_start_replace_all (g_active_editor,
            gtk_entry_get_text (gui->searchgui->searchentry),
            gtk_entry_get_text (gui->searchgui->replaceentry),
            gui->searchgui->backwards,
            gui->searchgui->wholeword,
            gui->searchgui->matchcase
            );
}
