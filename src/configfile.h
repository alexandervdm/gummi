/**
 * @file    configfile.h
 * @brief   handle configuration file
 *
 * Copyright (C) 2009 Gummi Developers
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

#ifndef __GUMMI_CONFIGFILE__
#define __GUMMI_CONFIGFILE__

#include <glib.h>


void config_init ();
void config_load_defaults ();
void config_save ();

// config get functions:
const gchar*   config_get_string  (const gchar* group, const gchar* key);
const gboolean config_get_boolean (const gchar* group, const gchar* key);
const gint     config_get_integer (const gchar* group, const gchar* key);

// config get default functions:
const gchar*   config_get_default_string  (const gchar* group, const gchar* key);
const gboolean config_get_default_boolean (const gchar* group, const gchar* key);
const gint     config_get_default_integer (const gchar* group, const gchar* key);

// config set functions:
void config_set_string  (const gchar *group, const gchar *key, gchar* value);
void config_set_boolean (const gchar *group, const gchar *key, gboolean value);
void config_set_integer (const gchar *group, const gchar *key, gint value);

// comparison functions:
gboolean config_value_as_str_equals (const gchar* group, const gchar* key, gchar* input);

#endif /* __GUMMI_CONFIGFILE__ */
