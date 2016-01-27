/**
 * @file    iofunctions.h
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


#ifndef __GUMMI_IOFUNCTIONS_H__
#define __GUMMI_IOFUNCTIONS_H__

#include "editor.h"

#include <glib.h>

#define GU_IOFUNC(x) ((GuIOFunc*)x)
typedef struct _GuIOFunc GuIOFunc;

struct _GuIOFunc {
  GObject* sig_hook;
};


/* Public functions */
GuIOFunc* iofunctions_init (void);
void iofunctions_load_default_text (gboolean loopedonce);
void iofunctions_load_file (GuIOFunc* io, const gchar* filename);
void iofunctions_save_file (GuIOFunc* io, gchar* filename, gchar *text);
gchar* iofunctions_get_swapfile (const gchar* filename);
gboolean iofunctions_has_swapfile (const gchar* filename);
void iofunctions_start_autosave (void);
void iofunctions_stop_autosave (void);
void iofunctions_reset_autosave (const gchar* name);
gboolean iofunctions_autosave_cb (void *user);

#endif /* __GUMMI_IOFUNCTIONS_H__ */
