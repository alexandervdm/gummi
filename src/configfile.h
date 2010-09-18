/**
 * @file    configfile.h
 * @brief   handle configuration file
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
 
#ifndef GUMMI_CONFIGFILE
#define GUMMI_CONFIGFILE

#include <stdio.h>

#include <glib.h>

#define BUF_MAX BUFSIZ / 8

/* Macros for handling configuration for both Gummi and Template */
#define CONFIG_NAME(type) ((0 == type)?config_filename: templcfg_filename)

#define config_init(NAME) configfile_init(NAME, 0)
#define config_set_default() configfile_set_default(0)
#define config_set_value(TERM, VALUE) configfile_set_value(0, TERM, VALUE)
#define config_get_value(TERM) configfile_get_value(0, TERM)

#define templcfg_init(NAME) configfile_init(NAME, 1)
#define templcfg_set_default() configfile_set_default(1)
#define templcfg_set_value(TERM, VALUE) configfile_set_value(1, TERM, VALUE)
#define templcfg_get_value(TERM) configfile_get_value(1, TERM)

typedef struct _slist {
    gchar line[BUF_MAX];
    struct _slist* next;
} slist;

/**
 * @brief initialize config file
 * @param filename filename of the configuration file
 */
void configfile_init(const gchar* filename, gint type);

/**
 * @brief reset settings to default
 */
void configfile_set_default(gint type);

/**
 * @brief get value of a setting
 * @param term the name of the setting
 * @return a pointer that points to the static gchar* of the setting value. If
 * the value type is boolean, configfile_get_value will return NULL for False
 * and non-NULL for True
 */
const gchar* configfile_get_value(gint type, const gchar* term);

/**
 * @brief set value of a setting
 * @param term the name of the setting
 * @param value the value of the setting
 */
void configfile_set_value(gint type, const gchar* term, const gchar* value);

/* [Internal] */
slist* configfile_load(gint type);
void configfile_save(gint type, slist* head);
slist* configfile_find_index_of(slist* head, const gchar* term);

#endif /* GUMMI_CONFIGFILE */
