/**
 * @file   project.h
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

#ifndef __GUMMI_PROJECT_H__
#define __GUMMI_PROJECT_H__

#include <glib.h>

typedef struct _Project {
    gchar* projfile;
    gchar* rootfile;

    int nroffiles;

} GuProject;


GuProject* project_init (void);
gboolean project_close (void);

gboolean project_create_new (const gchar* filename);
gboolean project_open_existing (const gchar* filename);

gboolean project_file_integrity (const gchar* content);
gboolean project_load_files (const gchar* projfile, const gchar* content);
GList* project_list_files (const gchar* content);
gchar* project_get_value (const gchar* content, const gchar* item);

gboolean project_add_document (const gchar* project, const gchar* fname);
gboolean project_remove_document (const gchar* project, const gchar* fname);

#endif /* __GUMMI_PROJECT_H__ */
