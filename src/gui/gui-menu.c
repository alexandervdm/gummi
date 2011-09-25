/**
 * @file   gui-menu.c
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

#include "gui-menu.h"

#include "configfile.h"
#include "environment.h"
#include "gui-main.h"
#include "updatecheck.h"

extern Gummi* gummi;
extern GummiGui* gui;

/* TODO: split the gui struct up into smaller pieces and try to remove all
 * non-gui functions and the "extern gummi" from this file */

void menugui_init (GtkBuilder* builder) {

}

/*******************************************************************************
 * FILE MENU                                                                   *
 ******************************************************************************/

 G_MODULE_EXPORT
void on_menu_new_activate (GtkWidget *widget, void* user) {
    if (!gtk_widget_get_sensitive (GTK_WIDGET (gui->rightpane)))
        gui_set_sensitive (TRUE);
    gui_create_environment (A_NONE, NULL, NULL);
}

G_MODULE_EXPORT
void on_menu_template_activate (GtkWidget *widget, void * user) {
    gtk_list_store_clear (gummi->templ->list_templates);
    template_setup (gummi->templ);
    gtk_widget_show_all (GTK_WIDGET (gummi->templ->templatewindow));
}

 G_MODULE_EXPORT
void on_menu_open_activate (GtkWidget *widget, void* user) {
    gchar *filename = NULL;

    if ( (filename = get_open_filename (TYPE_LATEX)))
        gui_open_file (filename);
    g_free (filename);

    if (g_active_editor)
        gtk_widget_grab_focus (GTK_WIDGET (g_active_editor->view));
}

G_MODULE_EXPORT
void on_menu_save_activate (GtkWidget *widget, void* user) {
    gui_save_file (FALSE);
}

G_MODULE_EXPORT
void on_menu_saveas_activate (GtkWidget *widget, void* user) {
    gui_save_file (TRUE);
}

G_MODULE_EXPORT
void on_menu_export_activate (GtkWidget *widget, void * user) {
    gchar* filename = NULL;

    filename = get_save_filename (TYPE_PDF);
    if (filename)
        latex_export_pdffile (gummi->latex, g_active_editor, filename, TRUE);
    g_free (filename);
}

G_MODULE_EXPORT
void on_menu_recent_activate (GtkWidget *widget, void * user) {
    const gchar* name = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));
    gchar* tstr;
    gint index = name[0] - '0' -1;

    if (utils_path_exists (gui->recent_list[index])) {
        gui_open_file (gui->recent_list[index]);
    } else {
        tstr = g_strdup_printf (_("Error loading recent file: %s"),
                gui->recent_list[index]);
        slog (L_ERROR, "%s\n", tstr);
        slog (L_G_ERROR, "Could not find the file %s.\n",
             gui->recent_list[index]);
        statusbar_set_message (tstr);
        g_free (tstr);
        g_free (gui->recent_list[index]);
        gui->recent_list[index] = NULL;
        while (index < RECENT_FILES_NUM -1) {
            gui->recent_list[index] = gui->recent_list[index+1];
            ++index;
        }
        gui->recent_list[RECENT_FILES_NUM -1] = g_strdup ("__NULL__");
    }
    display_recent_files (gui);
}

G_MODULE_EXPORT
void on_menu_close_activate (GtkWidget *widget, void* user) {
    gint ret = check_for_save ();
    GuTabContext* tab = NULL;

    if (GTK_RESPONSE_YES == ret)
        gui_save_file (FALSE);
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;

    tab = (user)? GU_TAB_CONTEXT (user): g_active_tab;

    if (!tabmanagergui_tab_pop (gui->tabmanagergui, tab)) {
        motion_start_errormode (gummi->motion, ""); // TODO: empty screen
        gui_set_sensitive (FALSE);
    } else
        gui_update_windowtitle ();
}

G_MODULE_EXPORT
gboolean on_menu_quit_activate (void) {
    gint wx = 0, wy = 0, width = 0, height = 0;
    gchar buf[16];
    int i = 0;

    gint length = g_list_length (gui->tabmanagergui->tabs);

    for(i = 0; i < length; i++){
        gtk_notebook_set_current_page(gui->tabmanagergui->notebook, i);
        tabmanagergui_set_active_tab(gui->tabmanagergui, i);

        gint ret = check_for_save ();
        if (GTK_RESPONSE_YES == ret)
            gui_save_file (FALSE);
        else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
            return TRUE;
    }

    gtk_window_get_size (gui->mainwindow, &width, &height);
    gtk_window_get_position (gui->mainwindow, &wx, &wy);
    config_set_value ("mainwindow_x", g_ascii_dtostr (buf, 16, (double)wx));
    config_set_value ("mainwindow_y", g_ascii_dtostr (buf, 16, (double)wy));
    config_set_value ("mainwindow_w", g_ascii_dtostr (buf, 16, (double)width));
    config_set_value ("mainwindow_h", g_ascii_dtostr (buf, 16, (double)height));

    gtk_main_quit ();
    /*
    for(i = 0; i < length; i++)
        editor_destroy (GU_TAB_CONTEXT (g_list_nth_data
                    (gui->tabmanagergui->tabs, i))->editor);
    */

    printf ("   ___ \n"
            "  {o,o}    Thanks for using Gummi!\n"
            "  |)__)    I welcome your feedback at:\n"
            "  -\"-\"-    http://gummi.midnightcoding.org\n\n");
    return FALSE;
}

/*******************************************************************************
 * EDIT MENU                                                                   *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_undo_activate (GtkWidget *widget, void* user) {
    editor_undo_change (g_active_editor);
}

G_MODULE_EXPORT
void on_menu_redo_activate (GtkWidget *widget, void* user) {
    editor_redo_change (g_active_editor);
}

G_MODULE_EXPORT
void on_menu_cut_activate (GtkWidget *widget, void* user) {
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard (g_e_buffer, clipboard, TRUE);
}

G_MODULE_EXPORT
void on_menu_copy_activate (GtkWidget *widget, void* user) {
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard (g_e_buffer, clipboard);
}
G_MODULE_EXPORT
void on_menu_paste_activate (GtkWidget *widget, void* user) {
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard (g_e_buffer, clipboard, NULL, TRUE);
}



G_MODULE_EXPORT
void on_menu_delete_activate (GtkWidget *widget, void * user) {
    gtk_text_buffer_delete_selection (g_e_buffer, FALSE, TRUE);
}

G_MODULE_EXPORT
void on_menu_selectall_activate (GtkWidget *widget, void * user) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (g_e_buffer, &start, &end);
    gtk_text_buffer_select_range (g_e_buffer, &start, &end);
}

G_MODULE_EXPORT
void on_menu_preferences_activate (GtkWidget *widget, void * user) {
    prefsgui_main (gui->prefsgui, 0);
}

/*******************************************************************************
 * VIEW MENU                                                                   *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_statusbar_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        gtk_widget_show (GTK_WIDGET (gui->statusbar));
        config_set_value ("statusbar", "True");
    } else {
        gtk_widget_hide (GTK_WIDGET (gui->statusbar));
        config_set_value ("statusbar", "False");
    }
}

G_MODULE_EXPORT
void on_menu_toolbar_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        gtk_widget_show (GTK_WIDGET (gui->toolbar));
        config_set_value ("toolbar", "True");
    } else {
        gtk_widget_hide (GTK_WIDGET (gui->toolbar));
        config_set_value ("toolbar", "False");
    }
}

G_MODULE_EXPORT
void on_menu_rightpane_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        gtk_widget_show (GTK_WIDGET (gui->rightpane));
        config_set_value ("rightpane", "True");
        gtk_toggle_tool_button_set_active (gui->previewoff, FALSE);
    } else {
        gtk_widget_hide (GTK_WIDGET (gui->rightpane));
        config_set_value ("rightpane", "False");
        gtk_toggle_tool_button_set_active (gui->previewoff, TRUE);
    }
}

G_MODULE_EXPORT
void on_menu_fullscreen_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)))
        gtk_window_fullscreen (gui->mainwindow);
    else
        gtk_window_unfullscreen (gui->mainwindow);
}

/*******************************************************************************
 * SEARCH MENU                                                                 *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_find_activate (GtkWidget *widget, void* user) {
    searchgui_main (gui->searchgui);
}

G_MODULE_EXPORT
void on_menu_findnext_activate (GtkWidget *widget, void * user) {
    editor_jumpto_search_result (g_active_editor, 1);
}

G_MODULE_EXPORT
void on_menu_findprev_activate (GtkWidget *widget, void * user) {
    editor_jumpto_search_result (g_active_editor, -1);
}

/*******************************************************************************
 * DOCUMENT MENU                                                               *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_bibinsert_activate (GtkWidget *widget, void * user) {
    gchar* filename = NULL;
    gchar* basename = NULL;
    gchar* root_path = NULL;
    gchar* relative_path = NULL;

    filename = get_open_filename (TYPE_BIBLIO);
    if (filename) {
        if (g_active_editor->filename)
            root_path = g_path_get_dirname (g_active_editor->filename);
        relative_path = utils_path_to_relative (root_path, filename);
        editor_insert_bib (g_active_editor, relative_path);
        basename = g_path_get_basename (filename);
        gtk_label_set_text (gummi->biblio->filenm_label, basename);
        g_free (relative_path);
        g_free (root_path);
        g_free (basename);
    }
    g_free (filename);
}

G_MODULE_EXPORT
void on_menu_bibcompile_activate (GtkWidget *widget, void * user) {
    //TODO: Merge with button function
    on_button_biblio_compile_clicked (widget, user);
}

 G_MODULE_EXPORT
void on_menu_pdfcompile_activate (GtkWidget *widget, void* user) {
    gummi->latex->modified_since_compile = TRUE;
    motion_do_compile (gummi->motion);
}

 G_MODULE_EXPORT
void on_menu_docstat_activate (GtkWidget *widget, void * user) {
    gint i = 0;
    gchar* output = 0;
    gchar* cmd = 0;
    gchar** matched = NULL;
    GError* err = NULL;
    GMatchInfo* match_info;
    GRegex* regexs[TEXCOUNT_OUTPUT_LINES];
    gchar* res[TEXCOUNT_OUTPUT_LINES] = { 0 };
    
    
    /* TODO: can we deprecate this? */
    const gchar* terms[] = {
        _("Words in text"),
        _("Words in headers"),
        _("Words in float captions"),
        _("Number of headers"),
        _("Number of floats"),
        _("Number of math inlines"),
        _("Number of math displayed")
    };
    
    const gchar* terms_regex[] = {
        "Words in text: ([0-9]*)",
        "Words in headers: ([0-9]*)",
        "Words in float captions: ([0-9]*)",
        "Number of headers: ([0-9]*)",
        "Number of floats: ([0-9]*)",
        "Number of math inlines: ([0-9]*)",
        "Number of math displayed: ([0-9]*)"
    };


    /* TODO: move to non gui class (latex perhaps) */
    if (utils_program_exists ("texcount")) {
        /* Copy workfile to /tmp to remove any spaces in filename to avoid
         * segfaults */
        gchar* tmpfile = g_strdup_printf ("%s.state", g_active_editor->fdname);
        if (!utils_copy_file (g_active_editor->workfile, tmpfile, &err)) {
            slog (L_G_ERROR, "utils_copy_file (): %s\n", err->message);
            g_free (tmpfile);
            g_error_free (err);
            goto cleanup;
        }

        cmd = g_strdup_printf ("texcount '%s'", tmpfile);
        Tuple2 result = utils_popen_r (cmd);

        for (i = 0; i < TEXCOUNT_OUTPUT_LINES; ++i)
            if (! (regexs[i] = g_regex_new (terms_regex[i], 0, 0, &err))) {
                slog (L_G_ERROR, "utils_copy_file (): %s\n", err->message);
                g_free (tmpfile);
                g_error_free (err);
                goto cleanup;
            }

        for (i = 0; i < TEXCOUNT_OUTPUT_LINES; ++i) {
            if (g_regex_match (regexs[i], result.second, 0, &match_info)) {
                matched = g_match_info_fetch_all (match_info);
                if (NULL == matched[1]) {
                    slog (L_WARNING, "can't extract info: %s\n", terms[i]);
                    res[i] = g_strdup ("N/A");
                } else {
                    res[i] = g_strdup (matched[1]);
                }
                g_strfreev (matched);
                g_match_info_free (match_info);
            }
        }
        g_free (result.second);
        g_free (tmpfile);
    }
    else {
        cmd = NULL;
        slog (L_G_ERROR, "The 'texcount' utility could not be found.\n");
        return;
    }

    gchararray items[6] = {"stats_words", "stats_head", "stats_float", 
                           "stats_nrhead", "stats_nrfloat", "stats_nrmath"};
    int j = 0;
    GtkLabel *tmp;
        
    for (j = 0; j < 6; j++) {
        gchar *value = items[j];
        tmp = GTK_LABEL(gtk_builder_get_object (gui->builder, value));
        gtk_label_set_text (tmp, res[j]);
    }
    
    /* TODO: make nice functions for retrieving tab labels */
    gtk_label_set_text (GTK_LABEL (gtk_builder_get_object (gui->builder, 
                    "stats_filename")), gtk_label_get_text(g_active_tabname));
    gtk_widget_show (gui->docstatswindow);
    return;

cleanup:
    for (i = 0; i < TEXCOUNT_OUTPUT_LINES; ++i) {
        g_regex_unref (regexs[i]);
        g_free (res[i]);
    }
    g_free (cmd);
    g_free (output);
}

G_MODULE_EXPORT
void on_menu_spelling_toggled (GtkWidget *widget, void * user) {
#ifdef USE_GTKSPELL
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        editor_activate_spellchecking (g_active_editor, TRUE);
        config_set_value ("spelling", "True");
    } else {
        editor_activate_spellchecking (g_active_editor, FALSE);
        config_set_value ("spelling", "False");
    }
#endif
}

G_MODULE_EXPORT
void on_menu_snippets_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        slog(L_INFO, "snippets activated\n");
        config_set_value ("snippets", "True");
    } else {
        slog(L_INFO, "snippets deactivated\n");
        config_set_value ("snippets", "False");
    }
}

/*******************************************************************************
 * PROJECT MENU                                                                *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_project_activate (GtkWidget *widget, void *user) {

    /* Only the menu items that are available from the current active
     * tab and environment should become sensitive */

    const gchar *save = _("Save the active tab to enable this option");
    //const gchar *invalid = _("The active tab is not a valid LaTeX document");
    //const gchar *detach = _("You cannot detach the top-level document");

    if (g_active_editor->filename != NULL) {
        gtk_widget_set_sensitive(GTK_WIDGET (gui->menu_include), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET (gui->menu_input), TRUE);
    }
    else {
        gtk_widget_set_tooltip_text(GTK_WIDGET (gui->menu_include), save);
        gtk_widget_set_tooltip_text(GTK_WIDGET (gui->menu_input), save);
    }
}

G_MODULE_EXPORT
void on_menu_project_deselect (GtkWidget *widget, void *user) {
    gtk_widget_set_sensitive(GTK_WIDGET (gui->menu_include), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET (gui->menu_input), FALSE);
}

G_MODULE_EXPORT
void on_menu_project_include_from_tab (GtkWidget *widget, void *user) {
    /* select a tab from a popup window with a liststore/treeview
     * file save dialog when selected top or slave file is not yet
     * saved" */

     /* write include command into the buffer at current position */
}

G_MODULE_EXPORT
void on_menu_project_include_new_file (GtkWidget *widget, void *user) {
    /* Create a new file and tab, popup with file save dialog */

    /* write include command into the buffer at current position */
}

G_MODULE_EXPORT
void on_menu_project_include_open_file (GtkWidget *widget, void *user) {

}

/*******************************************************************************
 * HELP MENU                                                                   *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_update_activate (GtkWidget *widget, void * user) {
    #ifdef WIN32
        slog (L_G_INFO, "To be implemented for win32..\n");
    #else
        gboolean ret = updatecheck (gui->mainwindow);
        if (!ret)
            slog (L_G_ERROR, "Update check failed!\n");
    #endif
}

G_MODULE_EXPORT
void on_menu_about_activate (GtkWidget *widget, void * user) {
    GError* err = NULL;
    gchar* icon_file = g_build_filename (DATADIR, "icons", "gummi.png", NULL);
    GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_size (icon_file, 80, 80, &err);
    g_free (icon_file);

    const gchar* authors[] = { "Alexander van der Mey\n"
        "<alexvandermey@gmail.com>",
        "Wei-Ning Huang\n"
        "<aitjcize@gmail.com>",
        "Dion Timmermann",
        "<dion.timmermann@tu-harburg.de>\n", 
        "Former contributors:",
        "Thomas van der Burgt",
        "Cameron Grout", NULL };

    const gchar* translators =
        "Arabic: Hamad Mohammad\n"
        "Brazilian-Portugese: Fernando Cruz & Alexandre Guimarães\n"
        "Catalan: Marc Vinyals\n"
        "Chinese (Traditional): Wei-Ning Huang\n"
        "Czech: Přemysl Janouch\n"
        "Danish: Jack Olsen\n"
        "Dutch: Alexander van der Mey\n"
        "French: Yvan Duron & Olivier Brousse\n"
        "German: Thomas Niederprüm\n"
        "Greek: Dimitris Leventeas\n"
        "Italian: Salvatore Vassallo\n"
        "Polish: Hubert Kowalski\n"
        "Portugese: Alexandre Guimarães\n"
        "Romanian: Alexandru-Eugen Ichim\n"
        "Russian: Kruvalig\n"
        "Spanish: Carlos Salas Contreras\n";

    GtkAboutDialog* dialog = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());
    gtk_window_set_transient_for (GTK_WINDOW (dialog), gui->mainwindow);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
    gtk_about_dialog_set_authors (dialog, authors);
    gtk_about_dialog_set_program_name (dialog, PACKAGE_NAME);
    gtk_about_dialog_set_version (dialog, PACKAGE_VERSION);
    gtk_about_dialog_set_website (dialog, PACKAGE_URL);
    gtk_about_dialog_set_copyright (dialog, PACKAGE_COPYRIGHT);
    gtk_about_dialog_set_license (dialog, PACKAGE_LICENSE);
    gtk_about_dialog_set_logo (dialog, icon);
    gtk_about_dialog_set_comments (dialog, PACKAGE_COMMENTS);
    gtk_about_dialog_set_translator_credits (dialog, translators);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));
}








