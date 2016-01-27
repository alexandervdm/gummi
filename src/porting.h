/**
 * @file   porting.h
 * @brief  Porting layer of different platforms
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

/* Fix GDK_KEY_foo for Gtk < 2.22 */
#include <gdk/gdkkeysyms.h>

#ifndef GDK_KEY_BackSpace
    #define GDK_KEY_BackSpace GDK_BackSpace
#endif

#ifndef GDK_KEY_Escape
    #define GDK_KEY_Escape GDK_Escape
#endif

#ifndef GDK_KEY_Tab
    #define GDK_KEY_Tab GDK_Tab
#endif

#ifndef GDK_KEY_Delete
    #define GDK_KEY_Delete GDK_Delete
#endif

#ifndef GDK_KEY_ISO_Left_Tab
    #define GDK_KEY_ISO_Left_Tab GDK_ISO_Left_Tab
#endif



