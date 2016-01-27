/**
 * @file    configfile.h
 * @brief   handle configuration file
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

#ifndef __GUMMI_CONFIGFILE__
#define __GUMMI_CONFIGFILE__

#include <stdio.h>

#include <glib.h>

void config_init (const gchar* filename);
void config_set_default (void);

/**
 * @brief get value of a setting
 * @param term the name of the setting
 * @return a pointer that points to the static gchar* of the setting value. If
 * the value type is boolean, config_get_value will return NULL for False
 * and non-NULL for True
 */
const gchar* config_get_value (const gchar* term);

/**
 * @brief set value of a setting, the settings will be write back immediately
 * @param term the name of the setting
 * @param value the value of the setting
 */
void config_set_value (const gchar* term, const gchar* value);

/**
 * @brief begin a series of config_set_value operation. The changes won't be
 * written back until config_commit() is called.
 */
void config_begin (void);

/**
 * @brief Terminate a series of config_set_value operation and write changes
 * back to the file.
 */
void config_commit (void);

void config_load (void);
void config_save (void);
void config_clean_up (void);

#endif /* __GUMMI_CONFIGFILE__ */
