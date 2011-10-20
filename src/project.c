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

#include <string.h>

#include "configfile.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "gui/gui-project.h"
#include "utils.h"

extern GummiGui* gui;
extern Gummi* gummi;


gboolean project_create_new (const gchar* filename) {
    const gchar* version = g_strdup ("0.6.0");
    const gchar* csetter = config_get_value ("typesetter");
    const gchar* csteps = config_get_value ("compile_steps");
    const gchar* rootfile = g_active_editor->filename;
    // TODO: do we need to encode this text?
    const gchar* content = g_strdup_printf("version=%s\n"
                                           "typesetter=%s\n"
                                           "steps=%s\n"
                                           "rootfile=%s\n", 
                                            version, csetter, csteps, rootfile);
    
    if (strcmp (filename + strlen (filename) -6, ".gummi")) {
        filename = g_strdup_printf ("%s.gummi", filename);
    }

    statusbar_set_message (g_strdup_printf("Creating project file: %s", filename));
    utils_set_file_contents (filename, content, -1);
    return TRUE;
}

gboolean project_open_existing (const gchar* filename) {
    gchar* content = NULL;
    GError* err = NULL;

    if (!g_file_get_contents (filename, &content, NULL, &err)) {
        slog (L_ERROR, "%s\n", err->message);
        return FALSE;
    }
    
    if (!project_file_integrity (content)) return FALSE;
    if (!project_load_files (filename, content)) return FALSE;
    
    return TRUE;
}

gboolean project_file_integrity (const gchar* content) {
    if (strlen (content) == 0) {
        return FALSE;
    }
    return TRUE;
}

gboolean project_add_document (const gchar* project, const gchar* fname) {
    return TRUE;
}

gboolean project_remove_document (const gchar* project, const gchar* fname) {
    return TRUE;
}

GList* project_list_files (const gchar* content) {
    gchar** splcontent = g_strsplit(content, "\n", 0);
    GList* filelist = NULL;
    gint i;
    
    for (i=0;i<g_strv_length(splcontent)-1;i++) {
        gchar** line = g_strsplit(splcontent[i], "=", 0);
        if (utils_strequal ("file", line[0])) {
            filelist = g_list_append (filelist, line[1]);
        }
        if (utils_strequal ("rootfile", line[0])) {
            filelist = g_list_prepend (filelist, line[1]);
        }
    }
    return filelist;
}
    
gboolean project_load_files (const gchar* projfile, const gchar* content) {
    
    gint rootpos, i;
    gchar* filename;

    GList* filelist = project_list_files (content);
    gint length = g_list_length (filelist);

    for (i=0; i<length;i++) {
        filename = g_list_nth_data (filelist, i);
        if (g_file_test (filename, G_FILE_TEST_EXISTS)) {
            
            //tabmanager_create_tab (A_LOAD, filename, NULL);
            gui_open_file (filename);
            
            // TODO: no direct calling this:
            g_active_editor->projfile = g_strdup (projfile);
        }
        if (i == 0) {
            rootpos = tabmanagergui_get_current_page ();
        }
    }
    projectgui_set_rootfile (rootpos);
    
    return TRUE;
}

gchar* project_get_value (const gchar* content, const gchar* item) {
    gchar** splcontent = g_strsplit(content, "\n", 0);
    gchar* result = g_strdup ("");
    gint i;
    
    for (i=0;i<g_strv_length(splcontent)-1;i++) {
        gchar** line = g_strsplit(splcontent[i], "=", 0);
        if (utils_strequal (item, line[0])) {
            return line[1];
        }
    }
    return result;
}




