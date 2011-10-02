/**
 * @file   project.h
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
 
#include "project.h"

#include "gui/gui-main.h"
#include "utils.h"

extern GummiGui* gui;
 
gboolean project_create_new (const gchar* filename) {
    //printf("creating new project file\n");
    return TRUE;
}

gboolean project_open_existing (const gchar* filename) {
    gint i, rootpos = NULL;
    gchar* content = NULL;
    GError* err = NULL;

    if (!g_file_get_contents (filename, &content, NULL, &err)) {
        slog (L_ERROR, "%s\n", err->message);
        return FALSE;
    }
    /* TODO: crude file analyser, needs more work on structure
     * so don't write a create project file yet! */
    gchar** lines = g_strsplit(content, "\n", 0);
    
    for (i=0;i<g_strv_length(lines)-1;i++) {
        gchar** line = g_strsplit(lines[i], "=", 0);
        
        if (utils_subinstr ("file", line[0], TRUE)) {
            if (g_file_test (line[1], G_FILE_TEST_EXISTS)) {
                gui_open_file (line[1]);
            }
            if (utils_subinstr ("rootfile", line[0], TRUE)) {
                rootpos = tabmanagergui_get_active_tab (gui->tabmanagergui);
            }
        }
    }
    tabmanagergui_switch_tab (gui->tabmanagergui, rootpos);
    return TRUE;
}


