/**
 * @file    tabmanager.h
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

#ifndef __GUMMI_TABMANAGER_H__
#define __GUMMI_TABMANAGER_H__

#include <glib.h>

#include "editor.h"

#include "gui/gui-tabmanager.h"

#define g_tabs gummi->tabmanager->tabs

typedef enum _OpenAct {
    A_NONE = 0,
    A_DEFAULT,
    A_LOAD,
    A_LOAD_OPT,
} OpenAct;

#define GU_TABMANAGER(x) ((GuTabmanager*)x)
typedef struct _GuTabmanager GuTabmanager;

struct _GuTabmanager {
    GuEditor* active_editor;
    GuTabContext* active_tab;
    GList* tabs;
};

GuTabmanager* tabmanager_init (void);

void tabmanager_foreach_editor (GFunc func, gpointer user_data);

gchar* tabmanager_get_tabname (GuTabContext* tc);
void tabmanager_set_active_tab (int position);

gboolean tabmanager_remove_tab (GuTabContext* tab);

/*------------------------------------------------------------------------*/

void tabmanager_create_tab (OpenAct act, const gchar* filename, gchar* opt);
void tabmanager_update_tab (const gchar* filename);
gboolean tabmanager_has_tabs ();
gboolean tabmanager_check_exists (const gchar* filename);

void tabmanager_set_content (OpenAct act, const gchar* filename, gchar* opt);

#endif /* __GUMMI_TABMANAGER_H__ */
