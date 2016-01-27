/**
 * @file   gui-menu.h
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

#ifndef __GUMMI_GUI_MENU_H__
#define __GUMMI_GUI_MENU_H__

#include <glib.h>
#include <gtk/gtk.h>

#define g_menugui gui->menugui

#define GU_MENU_GUI(x) ((GuMenuGui*)x)
typedef struct _GuMenuGui GuMenuGui;

struct _GuMenuGui {
    GtkMenuItem* menu_projcreate;
    GtkMenuItem* menu_projopen;
    GtkMenuItem* menu_projclose;
    GtkMenuItem* menu_cut;
    GtkMenuItem* menu_copy;
};

GuMenuGui* menugui_init (GtkBuilder* builder);
void on_menu_close_activate (GtkWidget *widget, void* user);

#ifdef WIN32
	void on_menu_donate_activate (GtkWidget *widget, void* user);
#endif

#endif /* __GUMMI_GUI_MENU_H__ */
