/**
 * @file   gui-infoscreen.h
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

#ifndef __GUMMI_GUI_ERRORSCREEN_H__
#define __GUMMI_GUI_ERRORSCREEN_H__

#include <gtk/gtk.h>

#define g_infoscreengui gui->infoscreengui

#define GU_INFOSCREEN_GUI(x) ((GuInfoscreenGui*)x)
typedef struct _GuInfoscreenGui GuInfoscreenGui;

struct _GuInfoscreenGui {
    GtkViewport* viewport;
    GtkWidget* errorpanel;
    GtkWidget* drawarea;

    GtkLabel *header;
    GtkImage *image;
    GtkLabel *details;
};

GuInfoscreenGui* infoscreengui_init (GtkBuilder* builder);
void infoscreengui_enable (GuInfoscreenGui *is, const gchar *msg);
void infoscreengui_disable (GuInfoscreenGui *is);
void infoscreengui_set_message (GuInfoscreenGui *is, const gchar *msg);


#endif /* __GUMMI_GUI_ERRORSCREEN_H__ */
