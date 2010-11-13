/**
 * @file   gui-search.c
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

#include "gui-search.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "environment.h"
#include "utils.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuSearchGui* searchgui_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GuSearchGui* s;
    s = (GuSearchGui*)g_malloc(sizeof(GuSearchGui));
    s->searchwindow =
        GTK_WIDGET(gtk_builder_get_object(builder, "searchwindow"));
    s->searchentry =
        GTK_ENTRY(gtk_builder_get_object(builder, "searchentry"));
    s->replaceentry =
        GTK_ENTRY(gtk_builder_get_object(builder, "replaceentry"));
    s->matchcase = TRUE;
    g_signal_connect(s->searchentry, "changed",
            G_CALLBACK(on_searchgui_text_changed), NULL);
    s->backwards = FALSE;
    s->wholeword = FALSE;
    return s;
}

void on_toggle_matchcase_toggled(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gui->searchgui->matchcase =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gummi->editor->replace_activated = FALSE;
}

void on_toggle_wholeword_toggled(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gui->searchgui->wholeword =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gummi->editor->replace_activated = FALSE;
}

void on_toggle_backwards_toggled(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gui->searchgui->backwards =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gummi->editor->replace_activated = FALSE;
}

void on_searchgui_text_changed(GtkEditable *editable, void* user) {
    L_F_DEBUG;
    gummi->editor->replace_activated = FALSE;
}
