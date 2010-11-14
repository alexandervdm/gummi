/**
 * @file    snippets.h
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
 
#ifndef GUMMI_SNIPPETS
#define GUMMI_SNIPPETS

#include <stdio.h>

#include <glib.h>

#include "editor.h"
#include "utils.h"

typedef struct _GuSnippets {
    GuEditor* b_editor;
    gchar* filename;
    slist* head;
} GuSnippets;

GuSnippets* snippets_init(const gchar* filename, GuEditor* ec);

/**
 * @brief reset settings to default
 */
void snippets_set_default(GuSnippets* sc);

/* [Internal] */
void snippets_load(GuSnippets* sc);
void snippets_save(GuSnippets* sc);
void snippets_clean_up(GuSnippets* sc);

#endif /* GUMMI_SNIPPETS */
