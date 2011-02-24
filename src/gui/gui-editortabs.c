/**
 * @file   gui-editortabs.c
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

#include "gui-editortabs.h"

#include <gtk/gtk.h>

#include "environment.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuEditortabsGui* editortabsgui_init(GtkBuilder* builder) {
    g_return_val_if_fail(GTK_IS_BUILDER(builder), NULL);
    
    GuEditortabsGui* et = g_new0(GuEditortabsGui, 1);
    
    GtkWidget *scroll = 
            GTK_WIDGET(gtk_builder_get_object(builder, "tab_default_scroll"));
    gtk_container_add(
            GTK_CONTAINER(scroll), GTK_WIDGET(gummi->editor->sourceview));

    return et;
}


GuEditor* get_active_editor(void) {
    return gummi->editor;
}

