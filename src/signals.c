/**
 * @file   signals.c
 * @brief  Define signals for Gummi
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

#include <glib.h>
#include <gtk/gtk.h>

void gummi_signals_register (void) {
    g_signal_new ("document-load",
            G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST,
            0, NULL, NULL,
            g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE, 1, G_TYPE_POINTER);

    g_signal_new ("document-write",
            G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST,
            0, NULL, NULL,
            g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE, 1, G_TYPE_POINTER);
}
