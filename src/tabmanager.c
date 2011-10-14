/**
 * @file   tabmanager.c
 * @brief  
 *
 * Copyright (C) 2010-2011 Gummi-Dev Team <alexvandermey@gmail.com>
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

#include "tabmanager.h"

#include "environment.h"

extern Gummi* gummi;

GuTabmanager* tabmanager_init (void) {
    GuTabmanager* tm = g_new0 (GuTabmanager, 1);
    
    tm->tabs = NULL;
    tm->active_tab = NULL; // ALEX: new
    return tm;
}

void tabmanager_foreach_editor (GFunc func, gpointer user_data) {
    GList* tabs = g_tabs;
    while (tabs) {
        GList *next = tabs->next;
        (*func) (GU_TAB_CONTEXT (tabs->data)->editor, user_data);
        tabs = next;
    }
}


