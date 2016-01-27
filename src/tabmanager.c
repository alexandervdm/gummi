/**
 * @file   tabmanager.c
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

#include "tabmanager.h"
#include "environment.h"

extern Gummi* gummi;
extern GummiGui* gui;

static gboolean current_tab_replaceable (OpenAct act);

GuTabmanager* tabmanager_init (void) {
    GuTabmanager* tm = g_new0 (GuTabmanager, 1);

    tm->tabs = NULL;
    tm->active_editor = NULL;
    tm->active_tab = NULL;

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

static gboolean current_tab_replaceable (OpenAct act) {
    if ((act == A_LOAD || act == A_LOAD_OPT))
        if (g_active_editor && !g_active_editor->filename)
            if (!editor_buffer_changed (g_active_editor))
                return TRUE;
    return FALSE;
}

gchar* tabmanager_get_tabname (GuTabContext* tc) {
    gchar* labeltext;
    gchar* filetext;
    gboolean modified;

    if (tc->editor->filename) {
        filetext = g_path_get_basename (tc->editor->filename);
    } else {
        filetext = g_strdup_printf (_("Unsaved Document %d"),
                                       tc->page->unsavednr);
    }

    modified = editor_buffer_changed (tc->editor);

    labeltext = g_strdup_printf ("%s%s", (modified? "*": ""), filetext);
    g_free (filetext);
    return labeltext;
}

gboolean tabmanager_remove_tab (GuTabContext* tab) {
    gint position = g_list_index (g_tabs, tab);
    gint total = tabmanagergui_get_n_pages ();

    if (total == 0) return FALSE;

    g_tabs = g_list_remove (g_tabs, tab);
    tabmanager_set_active_tab (total - 2);

    editor_destroy (tab->editor);
    gtk_notebook_remove_page (g_tabnotebook, position);
    g_free (tab);
    return (total != 1);
}

/*--------------------------------------------------------------------------*/


void tabmanager_set_active_tab (int position) {
    if (position == -1) {
        g_active_tab = NULL;
        g_active_editor = NULL;
    } else {
        g_active_tab = GU_TAB_CONTEXT (g_list_nth_data (g_tabs, position));
        g_active_editor =
            GU_TAB_CONTEXT (g_list_nth_data (g_tabs, position))->editor;
    }
}


void tabmanager_create_tab (OpenAct act, const gchar* filename, gchar* opt) {
    gint pos = 0;

    GuEditor* editor = gummi_new_environment (filename);

    if (current_tab_replaceable (act)) {
        pos = tabmanagergui_replace_page (g_active_tab, editor);
    }
    else {
        GuTabContext* tc = g_new0(GuTabContext, 1);
        tc->editor = editor;
        g_tabs = g_list_append(g_tabs, tc);
        pos = tabmanagergui_create_page (tc, tc->editor);
        tabmanagergui_set_current_page (pos);
    }

    tabmanager_set_active_tab (pos);

    if (iofunctions_has_swapfile (filename)) {
        gui_recovery_mode_enable (g_active_tab, filename);
        // signal handles tabmanager_set_content in this case..
    }
    else {
        tabmanager_set_content (act, filename, opt);
    }

    gui_set_filename_display (g_active_tab, TRUE, TRUE);
    add_to_recent_list (editor->filename);

    previewgui_reset (gui->previewgui);
}

void tabmanager_set_content (OpenAct act, const gchar* filename, gchar* opt) {
    // Loads the appropriate content in the editor. Seperated from
    // tab creation function to allow calling it from recovery mode
    switch (act) {
        case A_NONE:
            break;
        case A_DEFAULT:
            iofunctions_load_default_text (FALSE);
            break;
        case A_LOAD:
            iofunctions_load_file (gummi->io, filename);
            break;
        case A_LOAD_OPT:
            iofunctions_load_file (gummi->io, opt);
            break;
        default:
            slog(L_FATAL, "can't happen bug\n");
    }
}

void tabmanager_update_tab (const gchar* filename) {
    /* this function is called when the document is saved. no new editor
     * or tab object has to be initialised, but we'll need a fileinfo env
     * to match the new filename and its location and a gui update*/

    gui_set_filename_display (g_active_tab, TRUE, TRUE);

    editor_fileinfo_update (g_active_tab->editor, filename);

    // Add full filepath to recent list
    add_to_recent_list (g_active_tab->editor->filename);

    slog (L_INFO, "Environment updated for %s\n",
            g_active_tab->editor->filename);
    previewgui_reset (gui->previewgui);
}

gboolean tabmanager_has_tabs () {
    if (g_list_length (g_tabs) == 0) {
        if (g_active_editor != NULL) {
            slog (L_ERROR, "Something went terribly wrong in has_tabs\n");
        }
        return FALSE;
    }
    return TRUE;
}

gboolean tabmanager_check_exists (const gchar* filename) {
    GList *editors;
    GuEditor* ec;
    gint ectotal, i;

    editors = gummi_get_all_editors ();
    ectotal = g_list_length (editors);

    for (i=0; i<ectotal; i++) {
        ec = g_list_nth_data (editors, i);
        if (STR_EQU (ec->filename, filename)) return TRUE;
    }
    return FALSE;
}
