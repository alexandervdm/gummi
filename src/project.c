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

#include "project.h"

#include <string.h>

#include "configfile.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "gui/gui-project.h"
#include "utils.h"

// XXX: needs refactor, non gui classes should no directly acces gui and gummi
// structure
extern GummiGui* gui;
extern Gummi* gummi;

GuProject* project_init (void) {
    GuProject* p = g_new0 (GuProject, 1);

    p->projfile = NULL;
    p->rootfile = NULL;
    p->nroffiles = 1;

    return p;
}


gboolean project_create_new (const gchar* filename) {
    const gchar* version = g_strdup ("0.6.0");
    const gchar* csetter = config_get_value ("typesetter");
    const gchar* csteps = config_get_value ("compile_steps");
    const gchar* rootfile = g_active_editor->filename;
    // TODO: do we need to encode this text?
    const gchar* content = g_strdup_printf("version=%s\n"
                                           "typesetter=%s\n"
                                           "steps=%s\n"
                                           "root=%s\n",
                                            version, csetter, csteps, rootfile);

    if (!STR_EQU (filename + strlen (filename) -6, ".gummi")) {
        filename = g_strdup_printf ("%s.gummi", filename);
    }

    statusbar_set_message (g_strdup_printf("Creating project file: %s",
                filename));
    utils_set_file_contents (filename, content, -1);

    gummi->project->projfile = g_strdup (filename);

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

    gummi->project->projfile = g_strdup (filename);

    return TRUE;
}

gboolean project_close (void) {
    GList *tabs = NULL;
    int i = 0;
    tabs = g_list_copy(gummi_get_all_tabs());

    // XXX: needs refactor
    /* Disable compile thread to prevent it from compiling nonexisting editor */
    motion_stop_compile_thread(gummi->motion);
    tabmanager_set_active_tab(-1);

    for (i = 0; i < g_list_length (tabs); i++) {
        GuTabContext* tab = GU_TAB_CONTEXT (g_list_nth_data (tabs, i));
        if (tab->editor->projfile != NULL)
            on_menu_close_activate(NULL, tab);
    }
    g_list_free(tabs);

    /* Resume compile by selecting an active tag */
    if (gummi_get_all_tabs() != NULL)
        tabmanager_set_active_tab(0);
    motion_start_compile_thread(gummi->motion);

    return TRUE;
}

gboolean project_file_integrity (const gchar* content) {
    if (strlen (content) == 0) {
        return FALSE;
    }
    return TRUE;
}

gboolean project_add_document (const gchar* project, const gchar* fname) {
    gchar* oldcontent;
    gchar* newcontent;
    GError* err;

    if (!g_file_get_contents (project, &oldcontent, NULL, &err)) {
        slog (L_ERROR, "%s\n", err->message);
        return FALSE;
    }

    // don't add files that are already in the project:
    if (utils_subinstr ((gchar*)fname, oldcontent, TRUE)) return FALSE;

    newcontent = g_strconcat (oldcontent, "\nfile=", fname, NULL);

    if (g_file_test (project, G_FILE_TEST_EXISTS)) {
        utils_set_file_contents (project, newcontent, -1);
        return TRUE;
    }

    g_free(oldcontent);
    g_free(newcontent);

    return FALSE;
}

gboolean project_remove_document (const gchar* project, const gchar* fname) {
    gchar* oldcontent;
    gchar* newcontent;
    GError* err;

    if (!g_file_get_contents (project, &oldcontent, NULL, &err)) {
        slog (L_ERROR, "%s\n", err->message);
        return FALSE;
    }

    gchar* delimiter = g_strdup_printf ("file=%s", fname);

    gchar** splitcontent = g_strsplit (oldcontent, delimiter, 2);
    newcontent = g_strconcat (splitcontent[0], splitcontent[1], NULL);

    if (g_file_test (project, G_FILE_TEST_EXISTS)) {
        utils_set_file_contents (project, newcontent, -1);
        return TRUE;
    }

    g_free(oldcontent);
    g_free(newcontent);

    return FALSE;
}

GList* project_list_files (const gchar* content) {
    gchar** splcontent = g_strsplit(content, "\n", 0);
    GList* filelist = NULL;
    gint i;

    for (i = 0; i < g_strv_length(splcontent); i++) {
        gchar** line = g_strsplit(splcontent[i], "=", 0);
        if (STR_EQU ("file", line[0])) {
            filelist = g_list_append (filelist, line[1]);
            gummi->project->nroffiles += 1;
        }
        if (STR_EQU ("root", line[0])) {
            filelist = g_list_prepend (filelist, line[1]);
            gummi->project->rootfile = g_strdup (line[1]);
        }
    }
    return filelist;
}

gboolean project_load_files (const gchar* projfile, const gchar* content) {
    gboolean status = FALSE;
    gint rootpos, i;
    gchar* filename;

    GList* filelist = project_list_files (content);
    gint length = g_list_length (filelist);

    for (i=0; i<length;i++) {
        filename = g_list_nth_data (filelist, i);
        if (g_file_test (filename, G_FILE_TEST_EXISTS)) {

            if (!tabmanager_check_exists (filename)) {
                gui_open_file (filename);
                // TODO: no direct calling this:
                g_active_editor->projfile = g_strdup (projfile);
            }
            status = TRUE;
        }
        if (i == 0) {
            rootpos = tabmanagergui_get_current_page ();
        }
    }
    if (status == TRUE) projectgui_set_rootfile (rootpos);

    return status;
}

gchar* project_get_value (const gchar* content, const gchar* item) {
    gchar** splcontent = g_strsplit(content, "\n", 0);
    gchar* result = g_strdup ("");
    gint i;

    for (i = 0; i < g_strv_length(splcontent) -1; i++) {
        gchar** line = g_strsplit(splcontent[i], "=", 0);
        if (STR_EQU (item, line[0])) {
            return line[1];
        }
    }
    return result;
}
