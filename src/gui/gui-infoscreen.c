/**
 * @file    gui-infoscreen.c
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

#include "gui/gui-infoscreen.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "environment.h"
#include "gui/gui-tabmanager.h"
#include "gui/gui-main.h"

extern GummiGui* gui;

static const gchar* get_infoheader (int id);
static const gchar* get_infodetails (int id);

GuInfoscreenGui* infoscreengui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuInfoscreenGui* is = g_new0 (GuInfoscreenGui, 1);
    
    is->viewport =
        GTK_VIEWPORT (gtk_builder_get_object (builder, "preview_vport"));
    is->errorpanel =
        GTK_WIDGET(gtk_builder_get_object (builder, "errorpanel"));
    is->drawarea =
        GTK_WIDGET (gtk_builder_get_object (builder, "preview_draw"));
        
    is->header =
        GTK_LABEL (gtk_builder_get_object (builder, "error_header"));
    is->image =
        GTK_IMAGE (gtk_builder_get_object (builder, "error_image"));
    is->details =
        GTK_LABEL (gtk_builder_get_object (builder, "error_details"));
    return is;
}

void infoscreengui_enable (GuInfoscreenGui *is, const gchar *msg) {
    GList* list = NULL;
    
    list = gtk_container_get_children (GTK_CONTAINER (is->viewport));
    
    infoscreengui_set_message (is, msg);
    
    while (list) {
        gtk_container_remove (GTK_CONTAINER (is->viewport),
                GTK_WIDGET (list->data));
        list = list->next;
    }
    
    gtk_container_add (GTK_CONTAINER (is->viewport),
                GTK_WIDGET (is->errorpanel));
    gtk_widget_show_all (GTK_WIDGET (is->viewport));
    gtk_widget_set_sensitive (gui->previewgui->previewgui_toolbar, FALSE);
}

void infoscreengui_disable (GuInfoscreenGui *is) {
    
    g_object_ref (is->errorpanel);
    gtk_container_remove (GTK_CONTAINER (is->viewport),
            GTK_WIDGET (is->errorpanel));
    gtk_container_add (GTK_CONTAINER (is->viewport),
            GTK_WIDGET (is->drawarea));
    gtk_widget_set_sensitive (gui->previewgui->previewgui_toolbar, TRUE);
}

void infoscreengui_set_message (GuInfoscreenGui *is, const gchar *msg) {
    gtk_widget_set_visible (GTK_WIDGET(is->image), TRUE);
    if (STR_EQU (msg, "compile_error")) {
        gtk_label_set_text (is->header, get_infoheader(1));
        gtk_label_set_text (is->details, get_infodetails(1));
    }
    else if (STR_EQU (msg, "document_error")) {
        gtk_label_set_text (is->header, get_infoheader(2));
        gtk_label_set_text (is->details, get_infodetails(2));
    }
    else if (STR_EQU (msg, "program_error")) {
        gtk_label_set_text (is->header, get_infoheader(3));
        gtk_label_set_text (is->details, get_infodetails(3));
    }
    else {
        gtk_label_set_text (is->header, get_infoheader(4));
        gtk_label_set_text (is->details, get_infodetails(4));
        gtk_widget_set_visible (GTK_WIDGET(is->image), FALSE);
    }
}

static const gchar* get_infoheader (int id) {
    switch (id) {
        case 1: return _("PDF preview could not initialise.");
        case 2: return _("Document appears to be empty or invalid.");
        case 3: return _("Compilation program is missing.");
        case 4: return "";
        default: return "This should not have happened, bug!";
    }
}

static const gchar* get_infodetails (int id) {
    switch (id) {
        case 1: return _(
        "The active document contains errors. The live preview\n"
        "function will resume automatically once these errors\n"
        "are resolved. Additional information is available on\n"
        "the Build log tab.\n");
        case 2: return _(
        "The document that is currently active appears to be an\n"
        "an invalid LaTeX file. You can continue working on it,\n"
        "load the default text or use the Project menu to add\n"
        "it to an active project.\n");
        case 3: return _(
        "The selected compilation program could not be located.\n"
        "Please restore the program or select an alternative\n"
        "typesetter command from the Preferences menu. The\n"
        "live preview function will not resume until Gummi\n"
        "is restarted.\n");
        case 4: return ("");
        default: return "This should not have happened, bug!";
    }
}

