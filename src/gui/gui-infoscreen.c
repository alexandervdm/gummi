/**
 * @file    gui-infoscreen.c
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

#include "gui/gui-infoscreen.h"

#include <glib.h>
#include <gtk/gtk.h>

/* Current info/warning messages - TODO: i18n? */

const gchar *compile_error_h = "PDF preview could not initialise.";
const gchar *compile_error_d = "\
The active document contains errors. The live preview\n"
"function will resume automatically once these errors\n"
"are resolved. Additional information can be found on\n"
"the Error Output tab.";

// TODO: needs better wording
const gchar *document_error_h = "Document appears to be empty or invalid.\n";
const gchar *document_error_d = "\
The document that is currently active appears to be an\n"
"an invalid LaTeX file. You can continue working on it\n"
"or use the options below to add it to an existing\n"
"master document.\n";

// TODO: needs better wording
const gchar *program_error_h = "Compilation program is missing.";
const gchar *program_error_d = "\
The selected compilation program could not be located.\n"
"Please restore the program or select an alternative\n"
"typesetter command from the Preferences menu. The\n"
"live preview function will not resume until Gummi\n"
"is restarted.\n";


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

void infoscreengui_enable (GuInfoscreenGui *is, gchar *msg) {
    
    if (is->errormode) return;
    is->errormode = TRUE;
    
    if (g_strcmp0 (msg, "compile_error") == 0) {
        infoscreengui_set_message (is, compile_error_h, compile_error_d);
    }
    else if (g_strcmp0 (msg, "program_error") == 0) {
        infoscreengui_set_message (is, program_error_h, program_error_d);
    }
    else {
        infoscreengui_set_message (is, document_error_h, document_error_d);
    }
    
    
    gtk_container_remove (GTK_CONTAINER (is->viewport),
            GTK_WIDGET (is->drawarea));
    gtk_container_add (GTK_CONTAINER (is->viewport),
            GTK_WIDGET (is->errorpanel));
    gtk_widget_show_all (GTK_WIDGET (is->viewport));
}

void infoscreengui_disable (GuInfoscreenGui *is) {
    
    if (!is->errormode) return;
    is->errormode = FALSE;
    
    g_object_ref (is->errorpanel);
    gtk_container_remove (GTK_CONTAINER (is->viewport),
            GTK_WIDGET (is->errorpanel));
    gtk_container_add (GTK_CONTAINER (is->viewport),
            GTK_WIDGET (is->drawarea));
}

void infoscreengui_set_message (GuInfoscreenGui *is, 
                        const gchar *header, const gchar *details) {

    gtk_label_set_text (is->header, header);
    gtk_label_set_text (is->details, details);
}




