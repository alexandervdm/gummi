/**
 * @file   gui-editortabs.h
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

#ifndef __GUMMI_GUI_EDITORTABS_H__
#define __GUMMI_GUI_EDITORTABS_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "editor.h"

#define g_editortabsgui gui->editortabsgui
#define g_tabs_notebook gui->editortabsgui->tab_notebook

typedef struct _GuEditortabsGui {
    GuEditor* default_editor;
    GtkWidget *tab_notebook;
} GuEditortabsGui;




GuEditortabsGui* editortabsgui_init(GtkBuilder* builder);
void editortabsgui_create_tab(GuEditor* editor, gchar* filename);

#endif /* __GUMMI_GUI_EDITORTABS_H__ */
