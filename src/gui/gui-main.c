/**
 * @file    gui-main.c
 * @brief  
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

#include "gui-main.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef WIN32
#   include <unistd.h>
#endif

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "biblio.h"
#include "configfile.h"
#include "editor.h"
#include "environment.h"
#include "importer.h"
#include "updatecheck.h"
#include "utils.h"
#include "template.h"

extern Gummi* gummi;
extern GummiGui* gui;

/* Many of the functions in this file are based on the excellent GTK+
 * tutorials written by Micah Carrick that can be found on: 
 * http://www.micahcarrick.com/gtk-glade-tutorial-part-3.html */

GummiGui* gui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GtkWidget *hpaned;
    GtkWidget *errortext;
    gint wx = 0, wy = 0, width = 0, height = 0;

    GummiGui* g = g_new0 (GummiGui, 1);

    errortext = GTK_WIDGET (gtk_builder_get_object (builder, "errorfield"));
    g->mainwindow =
        GTK_WINDOW (gtk_builder_get_object (builder, "mainwindow"));
    g->toolbar =
        GTK_HBOX (gtk_builder_get_object (builder, "toolbar"));
    g->statusbar =
        GTK_STATUSBAR (gtk_builder_get_object (builder, "statusbar"));
    g->rightpane =
        GTK_VBOX (gtk_builder_get_object (builder, "rightpanebox"));
    g->previewoff = GTK_TOGGLE_TOOL_BUTTON (
            gtk_builder_get_object (builder, "tool_previewoff"));
    g->errorbuff =
        gtk_text_view_get_buffer (GTK_TEXT_VIEW (errortext));
    g->menu_spelling =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_spelling"));
    g->menu_snippets =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_snippets"));
    g->menu_toolbar =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_toolbar"));
    g->menu_statusbar =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_statusbar"));
    g->menu_rightpane =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_rightpane"));
    g->statusid =
        gtk_statusbar_get_context_id (GTK_STATUSBAR (g->statusbar), "Gummi");
    g->recent[0] =
        GTK_MENU_ITEM (gtk_builder_get_object (builder, "menu_recent1"));
    g->recent[1] =
        GTK_MENU_ITEM (gtk_builder_get_object (builder, "menu_recent2"));
    g->recent[2] =
        GTK_MENU_ITEM (gtk_builder_get_object (builder, "menu_recent3"));
    g->recent[3] =
        GTK_MENU_ITEM (gtk_builder_get_object (builder, "menu_recent4"));
    g->recent[4] =
        GTK_MENU_ITEM (gtk_builder_get_object (builder, "menu_recent5"));

    g->editortabsgui = editortabsgui_init (builder);
    g->importgui = importgui_init (builder);
    g->previewgui = previewgui_init (builder);
    g->searchgui = searchgui_init (builder);
    g->prefsgui = prefsgui_init (g->mainwindow);
    g->snippetsgui = snippetsgui_init (g->mainwindow);

    gchar* icon_file = g_build_filename (DATADIR, "icons", "icon.png", NULL);
    gtk_window_set_icon_from_file (g->mainwindow, icon_file, NULL);
    g_free (icon_file);
    gtk_window_resize (g->mainwindow,
                      atoi (config_get_value ("mainwindow_w")),
                      atoi (config_get_value ("mainwindow_h")));

    wx = atoi (config_get_value ("mainwindow_x"));
    wy = atoi (config_get_value ("mainwindow_y"));
    if (wx && wy)
      gtk_window_move (g->mainwindow, wx, wy);
    else
      gtk_window_set_position (g->mainwindow, GTK_WIN_POS_CENTER);

    PangoFontDescription* font_desc = 
        pango_font_description_from_string ("Monospace 8");
    gtk_widget_modify_font (errortext, font_desc);
    pango_font_description_free (font_desc);
    gtk_window_get_size (g->mainwindow, &width, &height);

    hpaned= GTK_WIDGET (gtk_builder_get_object (builder, "hpaned"));
    gtk_paned_set_position (GTK_PANED (hpaned), (width/2)); 

#ifndef USE_GTKSPELL
    gtk_widget_set_sensitive (GTK_WIDGET (g->menu_spelling), FALSE);
#else
    if (config_get_value ("spelling"))
        gtk_check_menu_item_set_active (g->menu_spelling, TRUE);
#endif
    if (config_get_value ("snippets")) {
        gtk_check_menu_item_set_active (g->menu_snippets, TRUE);
        gtk_widget_show (GTK_WIDGET (g->menu_snippets));
    }
    if (config_get_value ("toolbar")) {
        gtk_check_menu_item_set_active (g->menu_toolbar, TRUE);
        gtk_widget_show (GTK_WIDGET (g->toolbar));
    }

    if (config_get_value ("statusbar")) {
        gtk_check_menu_item_set_active (g->menu_statusbar, TRUE);
        gtk_widget_show (GTK_WIDGET (g->statusbar));
    }

    if (config_get_value ("rightpane")) {
        gtk_check_menu_item_set_active (g->menu_rightpane, TRUE);
        gtk_widget_show (GTK_WIDGET (g->rightpane));
    } else {
        config_set_value ("compile_status", "False");
        gtk_toggle_tool_button_set_active (g->previewoff, TRUE);
    }

    if (!config_get_value ("compile_status"))
        gtk_toggle_tool_button_set_active (g->previewoff, TRUE);

    g->recent_list[0] = g_strdup (config_get_value ("recent1"));
    g->recent_list[1] = g_strdup (config_get_value ("recent2"));
    g->recent_list[2] = g_strdup (config_get_value ("recent3"));
    g->recent_list[3] = g_strdup (config_get_value ("recent4"));
    g->recent_list[4] = g_strdup (config_get_value ("recent5"));

    display_recent_files (g);

    return g;
}

void gui_main (GtkBuilder* builder) {
    gtk_builder_connect_signals (builder, NULL);       
    g_signal_connect (g_e_buffer, "changed",
            G_CALLBACK (check_preview_timer), NULL);
    gtk_widget_show_all (GTK_WIDGET (gui->mainwindow));

    // TODO: SVN NOTICE 23 NOVEMBER - REMOVE ON 0.6.0 RELEASE
    // only want to show this once, and perhaps for future instances
    if (!strlen (config_get_value ("svnpopup"))) {
        GtkWidget *tmp =
            GTK_WIDGET (gtk_builder_get_object (builder, "svnpopup")); 
        gtk_widget_show (tmp);
        config_set_value ("svnpopup", "popped");
    }
    
    gtk_main ();
}

gboolean gui_quit (void) {
    gint wx = 0, wy = 0, width = 0, height = 0;
    gchar buf[16];
    gint ret = check_for_save ();

    if (GTK_RESPONSE_YES == ret)
        gui_save_file (FALSE);
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return TRUE;

    editor_destroy (gummi->editor);
    gtk_window_get_size (gui->mainwindow, &width, &height);
    gtk_window_get_position (gui->mainwindow, &wx, &wy);
    config_set_value ("mainwindow_x", g_ascii_dtostr (buf, 16, (double)wx));
    config_set_value ("mainwindow_y", g_ascii_dtostr (buf, 16, (double)wy));
    config_set_value ("mainwindow_w", g_ascii_dtostr (buf, 16, (double)width));
    config_set_value ("mainwindow_h", g_ascii_dtostr (buf, 16, (double)height));

    gtk_main_quit ();

    printf ("   ___ \n"
            "  {o,o}    Thanks for using Gummi!\n" 
            "  |)__)    I welcome your feedback at:\n"
            "  -\"-\"-    http://gummi.midnightcoding.org\n\n");
    return FALSE;
}

void gui_new_environment (const gchar* filename) {
    gummi_new_environment (gummi, filename);
    add_to_recent_list (filename);
    gui_update_title ();
    while (gtk_events_pending ()) gtk_main_iteration ();
    previewgui_reset (gui->previewgui);
    motion_start_timer (gummi->motion);
    
    int nrpages = gtk_notebook_get_n_pages (gui->editortabsgui->tab_notebook);
    if (nrpages != 0) { // change this, because we will want to close the only tab as well sometimes
        /*
        GuEditor *ed = editor_init (gummi->motion);
        // todo: append to glist
        editortabsgui_create_tab (ed, filename);
        gtk_widget_show_all (gui->mainwindow);
        */
        printf ("multi-tab code!\n");
    }
    else {
        editortabsgui_create_tab (gummi->editor, gummi->editor->filename);
    }
}

void gui_update_title (void) {
    gchar* basename = NULL;
    gchar* dirname = NULL;
    gchar* title = NULL;
    if (gummi->editor->filename) {
        basename = g_path_get_basename (gummi->editor->filename);
        dirname = g_path_get_dirname (gummi->editor->filename);
        title = g_strdup_printf ("%s%s (%s) - %s",
                (gtk_text_buffer_get_modified (g_e_buffer)? "*": ""),
                basename, dirname, PACKAGE_NAME);
        g_free (basename);
        g_free (dirname);
    } else
        title = g_strdup_printf ("%sUnsaved Document - %s",
                (gtk_text_buffer_get_modified (g_e_buffer)? "*": ""),
                PACKAGE_NAME);

    gtk_window_set_title (gui->mainwindow, title);
    g_free (title);
}

void gui_open_file (const gchar* filename) {
    gint ret = 0;
    gchar* basename = g_path_get_basename (filename);
    gchar* dirname = g_path_get_dirname (filename);
    gchar* prev_workfile = g_strdup_printf ("%s%c.%s.swp", dirname,
            G_DIR_SEPARATOR, basename);

    /* destroy previous file info context, be careful not to place this
     * line before the previewgui_stop_preview, else if the user is using
     * the real_time compile scheme, the compile scheme functions can
     * access fileinfo */
    previewgui_stop_preview (gui->previewgui);
    editor_fileinfo_cleanup (gummi->editor);

    /* Check if swap file exists and try to recover from it */
    if (utils_path_exists (prev_workfile)) {
        slog (L_WARNING, "Swap file `%s' found.\n", prev_workfile);
        gchar* message = g_strdup_printf ("Swap file exits for %s, do you "
                "want to recover from it?", filename);

        ret = utils_yes_no_dialog (message);
        if (GTK_RESPONSE_YES == ret)
            iofunctions_load_file (gummi->io, prev_workfile); 
        g_free (message);
    }

    g_free (dirname);
    g_free (basename);
    g_free (prev_workfile);

    if (GTK_RESPONSE_YES != ret)
        iofunctions_load_file (gummi->io, filename); 

    gui_new_environment (filename);
}

void gui_save_file (gboolean saveas) {
    gboolean new = FALSE;
    gchar* filename = NULL;
    gchar* pdfname = NULL;
    gchar* prev = NULL;
    gint ret = 0;

    if (saveas || ! (filename = gummi->editor->filename)) {
        if ( (filename = get_save_filename (TYPE_LATEX))) {
            new = TRUE;
            if (strcmp (filename + strlen (filename) -4, ".tex")) {
                prev = filename;
                filename = g_strdup_printf ("%s.tex", filename);
                g_free (prev);
            }
            if (utils_path_exists (filename)) {
                ret = utils_yes_no_dialog (
                        _("The file already exists. Overwrite?"));
                if (GTK_RESPONSE_YES != ret) goto cleanup;
            }
        } else goto cleanup;
    }
    iofunctions_save_file (gummi->io, filename);

    pdfname = g_strdup (filename);
    pdfname[strlen (pdfname) -4] = 0;
    latex_export_pdffile (gummi->latex, gummi->editor, pdfname, FALSE);
    if (new) gui_new_environment (filename);
    gui_update_title ();
    gtk_widget_grab_focus (GTK_WIDGET (gummi->editor->view));

cleanup:
    if (new) g_free (filename);
    g_free (pdfname);
}

void on_menu_new_activate (GtkWidget *widget, void* user) {
    gint ret = check_for_save ();
    if (GTK_RESPONSE_YES == ret)
        gui_save_file (FALSE);
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;
    gui_new_environment (NULL);
    iofunctions_load_default_text ();
}

void on_menu_template_activate (GtkWidget *widget, void * user) {
    gtk_list_store_clear (gummi->templ->list_templates);
    template_setup (gummi->templ);
    gtk_widget_show_all (GTK_WIDGET (gummi->templ->templatewindow));
}

void on_menu_exportpdf_activate (GtkWidget *widget, void * user) {
    gchar* filename = NULL;

    filename = get_save_filename (TYPE_PDF);
    if (filename)
        latex_export_pdffile (gummi->latex, gummi->editor, filename, TRUE);
    g_free (filename);
}

void on_menu_recent_activate (GtkWidget *widget, void * user) {
    const gchar* name = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));
    gchar* tstr;
    gint index = name[0] - '0' -1;
    gint ret = check_for_save ();

    if (GTK_RESPONSE_YES == ret)
        gui_save_file (FALSE);
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;

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

void on_menu_open_activate (GtkWidget *widget, void* user) {
    gchar *filename = NULL;
    gint ret = check_for_save ();

    if (GTK_RESPONSE_YES == ret)
        gui_save_file (FALSE);
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;

    if ( (filename = get_open_filename (TYPE_LATEX)))
        gui_open_file (filename);
    g_free (filename);

    gtk_widget_grab_focus (GTK_WIDGET (gummi->editor->view));
}

void on_menu_save_activate (GtkWidget *widget, void* user) {
    gui_save_file (FALSE);
}

void on_menu_saveas_activate (GtkWidget *widget, void* user) {
    gui_save_file (TRUE);
}

void on_menu_cut_activate (GtkWidget *widget, void* user) {
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard (g_e_buffer, clipboard, TRUE);
}

void on_menu_copy_activate (GtkWidget *widget, void* user) {
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard (g_e_buffer, clipboard);
}
void on_menu_paste_activate (GtkWidget *widget, void* user) {
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard (g_e_buffer, clipboard, NULL, TRUE);
}

void on_menu_undo_activate (GtkWidget *widget, void* user) {
    editor_undo_change (gummi->editor);
}

void on_menu_redo_activate (GtkWidget *widget, void* user) {
    editor_redo_change (gummi->editor);
}

void on_menu_delete_activate (GtkWidget *widget, void * user) {
    gtk_text_buffer_delete_selection (g_e_buffer, FALSE, TRUE);
}

void on_menu_selectall_activate (GtkWidget *widget, void * user) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (g_e_buffer, &start, &end);
    gtk_text_buffer_select_range (g_e_buffer, &start, &end);
}

void on_menu_preferences_activate (GtkWidget *widget, void * user) {
    prefsgui_main (gui->prefsgui);
}

void on_menu_statusbar_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        gtk_widget_show (GTK_WIDGET (gui->statusbar));
        config_set_value ("statusbar", "True");
    } else {
        gtk_widget_hide (GTK_WIDGET (gui->statusbar));
        config_set_value ("statusbar", "False");
    }
}

void on_menu_toolbar_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        gtk_widget_show (GTK_WIDGET (gui->toolbar));
        config_set_value ("toolbar", "True");
    } else {
        gtk_widget_hide (GTK_WIDGET (gui->toolbar));
        config_set_value ("toolbar", "False");
    }
}

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

    void on_menu_fullscreen_toggled (GtkWidget *widget, void * user) {
        if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)))
            gtk_window_fullscreen (gui->mainwindow);
        else
            gtk_window_unfullscreen (gui->mainwindow);
    }

void on_menu_find_activate (GtkWidget *widget, void* user) {
    searchgui_main (gui->searchgui);
}

void on_menu_findnext_activate (GtkWidget *widget, void * user) {
    editor_jumpto_search_result (gummi->editor, 1);
}

void on_menu_findprev_activate (GtkWidget *widget, void * user) {
    editor_jumpto_search_result (gummi->editor, -1);
}

void on_menu_bibload_activate (GtkWidget *widget, void * user) {
    gchar* filename = NULL;
    gchar* basename = NULL;
    gchar* root_path = NULL;
    gchar* relative_path = NULL;

    filename = get_open_filename (TYPE_BIBLIO);
    if (filename) {
        if (gummi->editor->filename)
            root_path = g_path_get_dirname (gummi->editor->filename);
        relative_path = utils_path_to_relative (root_path, filename);
        editor_insert_bib (gummi->editor, relative_path);
        basename = g_path_get_basename (filename);
        gtk_label_set_text (gummi->biblio->filenm_label, basename);
        g_free (relative_path);
        g_free (root_path);
        g_free (basename);
    }
    g_free (filename);
}

void on_menu_bibupdate_activate (GtkWidget *widget, void * user) {
    biblio_compile_bibliography (gummi->biblio, gummi->editor, gummi->latex);
}

void on_menu_pdfcompile_activate (GtkWidget *widget, void* user) {
    gummi->latex->modified_since_compile = TRUE;
    motion_do_compile (gummi->motion);
}

void on_menu_docstat_activate (GtkWidget *widget, void * user) {
    GtkWidget* dialog = 0;
    gint i = 0;
    gchar* output = 0;
    gchar* cmd = 0;
    gchar** matched = NULL;
    GError* err = NULL;
    GMatchInfo* match_info;
    GRegex* regexs[TEXCOUNT_OUTPUT_LINES];
    gchar* res[TEXCOUNT_OUTPUT_LINES] = { 0 };
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
    
    if (g_find_program_in_path ("texcount")) {
        /* Copy workfile to /tmp to remove any spaces in filename to avoid
         * segfaults */
        gchar* tmpfile = g_strdup_printf ("%s.state", gummi->editor->fdname);
        if (!utils_copy_file (gummi->editor->workfile, tmpfile, &err)) {
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
    
        output = g_strconcat (terms[0], ": ", res[0], "\n",
                             terms[1], ": ", res[1], "\n",
                             terms[2], ": ", res[2], "\n",
                             terms[3], ": ", res[3], "\n",
                             terms[4], ": ", res[4], "\n",
                             terms[4], ": ", res[5], "\n",
                             terms[6], ": ", res[6], "\n",
                             NULL);
        g_free (result.second);
        g_free (tmpfile);
    }
    else {
        cmd = NULL;
        output = g_strdup (_("This function requires\nthe texcount program.\n"));
    }
    
    dialog = gtk_message_dialog_new (gui->mainwindow,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "%s", output);
    gtk_window_set_title (GTK_WINDOW (dialog), _("Document Statistics"));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

cleanup:
    for (i = 0; i < TEXCOUNT_OUTPUT_LINES; ++i) {
        g_regex_unref (regexs[i]);
        g_free (res[i]);
    }
    g_free (cmd);
    g_free (output);
}

void on_menu_spelling_toggled (GtkWidget *widget, void * user) {
#ifdef USE_GTKSPELL
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        editor_activate_spellchecking (gummi->editor, TRUE);
        config_set_value ("spelling", "True");
    } else {
        editor_activate_spellchecking (gummi->editor, FALSE);
        config_set_value ("spelling", "False");
    }
#endif
}

void on_menu_snippets_toggled (GtkWidget *widget, void * user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        slog(L_INFO, "snippets activated\n");
        config_set_value ("snippets", "True");
    } else {
        slog(L_INFO, "snippets deactivated\n");
        config_set_value ("snippets", "False");
    }
}

void on_menu_update_activate (GtkWidget *widget, void * user) {
    gboolean ret = updatecheck (gui->mainwindow);
    if (!ret)
        slog (L_G_ERROR, "Update check failed!\n");
}

void on_menu_about_activate (GtkWidget *widget, void * user) {
    GError* err = NULL;
    gchar* icon_file = g_build_filename (DATADIR, "icons", "gummi.png", NULL);
    GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_size (icon_file, 80, 80, &err);
    g_free (icon_file);

    const gchar* authors[] = { "Alexander van der Mey\n"
        "<alexvandermey@gmail.com>",
        "Wei-Ning Huang\n"
            "<aitjcize@gmail.com>\n",
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

void on_tool_previewoff_toggled (GtkWidget *widget, void * user) {
    gboolean value =
        gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (widget));
    config_set_value ("compile_status", (!value)? "True": "False");
    if (value)
        previewgui_stop_preview (gui->previewgui);
    else
        previewgui_start_preview (gui->previewgui);
}

void on_tool_textstyle_bold_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (gummi->editor, "tool_bold");
}

void on_tool_textstyle_italic_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (gummi->editor, "tool_italic");
}

void on_tool_textstyle_underline_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (gummi->editor, "tool_unline");
}

void on_tool_textstyle_left_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (gummi->editor, "tool_left");
}

void on_tool_textstyle_center_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (gummi->editor, "tool_center");
}

void on_tool_textstyle_right_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (gummi->editor, "tool_right");
}



void on_button_template_add_clicked (GtkWidget* widget, void* user) {
    template_add_new_entry (gummi->templ);
}

void on_button_template_remove_clicked (GtkWidget* widget, void* user) {
    template_remove_entry (gummi->templ);
}

void on_button_template_open_clicked (GtkWidget* widget, void* user) {
    gchar* status;
    templdata template = template_open_selected (gummi->templ);
    
    if (template.itemdata) {
        /* add Loading message to status bar */
        status = g_strdup_printf ("Loading template %s...", template.itemname);
        statusbar_set_message (status);
        g_free (status);
        
        gui_new_environment (NULL);
        editor_fill_buffer (gummi->editor, template.itemdata);
        gtk_widget_hide (GTK_WIDGET (gummi->templ->templatewindow));
    }
}

void on_button_template_close_clicked (GtkWidget* widget, void* user) {
    gtk_widget_set_sensitive (GTK_WIDGET (gummi->templ->template_add), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (gummi->templ->template_remove), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (gummi->templ->template_open), TRUE);
    gtk_widget_hide (GTK_WIDGET (gummi->templ->templatewindow));
}

void on_template_rowitem_edited (GtkWidget* widget, gchar *path, gchar* filenm,
        void* user) {
    GtkTreeIter iter;
    GtkTreeModel* model = NULL;
    GtkTreeSelection* selection = NULL;
    gchar* text = NULL;
    gchar* filepath = g_build_filename (g_get_user_config_dir (),
            "gummi", "templates", filenm, NULL);
    
    model = gtk_tree_view_get_model (gummi->templ->templateview);
    selection = gtk_tree_view_get_selection (gummi->templ->templateview);
    
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_list_store_set (gummi->templ->list_templates, &iter, 0, filenm, 1,
                filepath, -1);
        text = editor_grab_buffer (gummi->editor);
        template_create_file (gummi->templ, filenm, text);
    }
    g_free (text);
    g_free (filepath);
}

void on_bibcolumn_clicked (GtkWidget* widget, void* user) {
    gint id = gtk_tree_view_column_get_sort_column_id
        (GTK_TREE_VIEW_COLUMN (widget));
    gtk_tree_view_column_set_sort_column_id
        (GTK_TREE_VIEW_COLUMN (widget), id);
}

void on_button_biblio_compile_clicked (GtkWidget* widget, void* user) {
    gummi->biblio->progressval = 0.0;
    g_timeout_add (10, on_bibprogressbar_update, NULL);

    if (biblio_compile_bibliography (gummi->biblio, gummi->editor,
                gummi->latex)) {
        statusbar_set_message (_("Compiling bibliography file..."));
        gtk_progress_bar_set_text (gummi->biblio->progressbar,
                _("bibliography compiled without errors"));
    } else {
        statusbar_set_message (_("Error compiling bibliography file or none "
                    "detected..."));
        gtk_progress_bar_set_text (gummi->biblio->progressbar,
                _("error compiling bibliography file"));
    }
    check_preview_timer ();
}

void on_button_biblio_detect_clicked (GtkWidget* widget, void* user) {
    gchar* text = 0;
    gchar* str = 0;
    gchar* basename = 0;
    GError* err = NULL;
    gint number = 0;

    gummi->biblio->progressval = 0.0;
    g_timeout_add (2, on_bibprogressbar_update, NULL);
    gtk_list_store_clear (gummi->biblio->list_biblios);

    if (biblio_detect_bibliography (gummi->biblio, gummi->editor)) {
        editor_insert_bib (gummi->editor, gummi->editor->bibfile);
        if (!g_file_get_contents (gummi->editor->bibfile, &text, NULL, &err)) {
            slog (L_G_ERROR, "g_file_get_contents (): %s\n", err->message);
            g_error_free (err);
            return;
        }
        
        gtk_widget_set_sensitive 
                    (GTK_WIDGET(gummi->biblio->list_filter), TRUE);
        
        number = biblio_parse_entries (gummi->biblio, text);
        basename = g_path_get_basename (gummi->editor->bibfile);
        gtk_label_set_text (gummi->biblio->filenm_label, basename);
        str = g_strdup_printf ("%d", number);
        gtk_label_set_text (gummi->biblio->refnr_label, str);
        g_free (str);
        str = g_strdup_printf (_("%s loaded"), basename);
        gtk_progress_bar_set_text (gummi->biblio->progressbar, str);
        g_free (basename);
        g_free (str);
    }
    else {
        gtk_widget_set_sensitive 
                    (GTK_WIDGET(gummi->biblio->list_filter), FALSE);
        gtk_progress_bar_set_text (gummi->biblio->progressbar,
                _("no bibliography file detected"));
        gtk_label_set_text (gummi->biblio->filenm_label, _("none"));
        gtk_label_set_text (gummi->biblio->refnr_label, _("N/A"));
    }
}

void on_bibreference_clicked (GtkTreeView* view, GtkTreePath* Path,
        GtkTreeViewColumn* column, void* user) {
    GtkTreeIter iter;
    gchar* value;
    gchar* out;
    GtkTreeModel* model = GTK_TREE_MODEL (gummi->biblio->list_biblios);
    GtkTreeSelection* selection = gtk_tree_view_get_selection (view);

    gtk_tree_selection_get_selected (selection, &model, &iter);
    gtk_tree_model_get (model, &iter, 0, &value, -1);
    out = g_strdup_printf ("\\cite{%s}", value);
    gtk_text_buffer_insert_at_cursor (g_e_buffer, out, strlen (out));
    g_free (out);
}

static gboolean visible_func (GtkTreeModel *model, GtkTreeIter  *iter, gpointer data) {
    gboolean row_visible = FALSE;
    gchar* title;
    gchar* author;
    gchar* year;

    /* make all entries visible again when filter buffer is empty */
    if (strlen(data) == 0) return TRUE;

    gtk_tree_model_get (model, iter, 1, &title, 2, &author, 3, &year, -1);
    if (utils_subinstr (data, title , TRUE)) row_visible = TRUE;
    if (utils_subinstr (data, author , TRUE)) row_visible = TRUE;
    if (utils_subinstr (data, year , TRUE)) row_visible = TRUE;
    
    g_free(title);
    g_free(author);
    g_free(year);
    
    return row_visible;
}


void on_biblio_filter_changed (GtkWidget* widget, void* user) {
    GtkTreeModel *filter;

    gchar *entry = gtk_entry_get_text(GTK_ENTRY(widget));
    filter = gtk_tree_model_filter_new (
            GTK_TREE_MODEL(gummi->biblio->list_biblios),NULL);
    gtk_tree_model_filter_set_visible_func(
            GTK_TREE_MODEL_FILTER(filter), visible_func, entry, NULL);
    gtk_tree_view_set_model ( 
            GTK_TREE_VIEW( gummi->biblio->biblio_treeview ),filter);
    g_object_unref (G_OBJECT(filter));
}



gboolean on_bibprogressbar_update (void* user) {
    gtk_adjustment_set_value
        (gummi->biblio->progressmon, gummi->biblio->progressval);
    gummi->biblio->progressval += 1.0;
    return ! (gummi->biblio->progressval > 60);
}

gint check_for_save (void) {
    gint ret = 0;

    if (gtk_text_buffer_get_modified (g_e_buffer))
        ret = utils_yes_no_dialog (
                _("Do you want to save the changes you have made?"));
    return ret;
}

gchar* get_open_filename (GuFilterType type) {
    GtkFileChooser* chooser = NULL;
    static gchar* last_filename = NULL;
    gchar* filename = NULL;

    const gchar* chooser_title[] = {
        _("Open LaTeX document"),
        "shouldn't happen",
        "shouldn't happen",
        _("Select an image to insert"),
        _("Select bibliography file")
    };

    chooser = GTK_FILE_CHOOSER (gtk_file_chooser_dialog_new (
                chooser_title[type],
                gui->mainwindow,
                GTK_FILE_CHOOSER_ACTION_OPEN,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                NULL));

    file_dialog_set_filter (chooser, type);
    if (last_filename)
        gtk_file_chooser_set_current_folder (chooser, last_filename);
    else
        gtk_file_chooser_set_current_folder (chooser, g_get_home_dir ());

    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

    if (filename) {
        g_free (last_filename);
        last_filename = g_path_get_dirname (filename);
    }

    gtk_widget_destroy (GTK_WIDGET (chooser));
    return filename;
}

gchar* get_save_filename (GuFilterType type) {
    GtkFileChooser* chooser = NULL;
    gchar* filename = NULL;

    const gchar* chooser_title[] = {
        _("Save LaTeX document"),
        _("Save as LaTeX document"),
        _("Export to PDF"),
        "shouldn't happen",
        "shouldn't happen"
    };

    chooser = GTK_FILE_CHOOSER (gtk_file_chooser_dialog_new (
                chooser_title[type],
                gui->mainwindow,
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_OK,
                NULL));

    file_dialog_set_filter (chooser, type);
    gtk_file_chooser_set_current_folder (chooser, g_get_home_dir ());

    if (gummi->editor->filename) {
        gchar* dirname = g_path_get_dirname (gummi->editor->filename);
        gchar* basename = g_path_get_basename (gummi->editor->filename);

        gtk_file_chooser_set_current_folder (chooser, dirname);

        if (TYPE_PDF == type) {
            basename[strlen (basename) -4] = 0;
            gchar* path = g_strdup_printf ("%s.pdf", basename);
            gtk_file_chooser_set_current_name (chooser, path);
            g_free (path);
        } else if (TYPE_LATEX_SAVEAS == type)
            gtk_file_chooser_set_current_name (chooser, basename);

        g_free (dirname);
        g_free (basename);
    }

    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

    gtk_widget_destroy (GTK_WIDGET (chooser));
    return filename;
}

void file_dialog_set_filter (GtkFileChooser* dialog, GuFilterType type) {
    GtkFileFilter* filter = gtk_file_filter_new ();

    switch (type) {
        case TYPE_LATEX:
        case TYPE_LATEX_SAVEAS:
            gtk_file_filter_set_name (filter, "LaTeX files");
            gtk_file_filter_add_pattern (filter, "*.tex");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            filter = gtk_file_filter_new ();
            gtk_file_filter_set_name (filter, "Text files");
            gtk_file_filter_add_mime_type (filter, "text/plain");
            gtk_file_chooser_add_filter (dialog, filter);
            break;

        case TYPE_PDF:
            gtk_file_filter_set_name (filter, "PDF files");
            gtk_file_filter_add_pattern (filter, "*.pdf");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            break;

        case TYPE_IMAGE:
            gtk_file_filter_set_name (filter, "Image files");
            gtk_file_filter_add_mime_type (filter, "image/*");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            break;

        case TYPE_BIBLIO:
            gtk_file_filter_set_name (filter, "Bibtex files");
            gtk_file_filter_add_pattern (filter, "*.bib");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            break;
    }

}

void add_to_recent_list (const gchar* filename) {
    if (!filename) return;
    gint i = 0;
    /* check if it already exists */
    for (i = 0; i < 5; ++i)
        if (0 == strcmp (filename, gui->recent_list[i]))
            return;

    /* add to recent list */
    g_free (gui->recent_list[RECENT_FILES_NUM -1]);
    for (i = RECENT_FILES_NUM -2; i >= 0; --i)
        gui->recent_list[i + 1] = gui->recent_list[i];
    gui->recent_list[0] = g_strdup (filename);
    display_recent_files (gui);
}

void display_recent_files (GummiGui* gui) {
    gchar* tstr = 0;
    gchar* basename = 0;
    gint i = 0, count = 0;

    for (i = 0; i < 5; ++i)
        gtk_widget_hide (GTK_WIDGET (gui->recent[i]));

    for (i = 0; i < RECENT_FILES_NUM; ++i) {
        if (0 != strcmp (gui->recent_list[i], "__NULL__")) {
            basename = g_path_get_basename (gui->recent_list[i]);
            tstr = g_strdup_printf ("%d. %s", count + 1, basename);
            gtk_menu_item_set_label (gui->recent[i], tstr);
            gtk_widget_set_tooltip_text (GTK_WIDGET (gui->recent[i]),
                                        gui->recent_list[i]);
            gtk_widget_show (GTK_WIDGET (gui->recent[i]));
            g_free (tstr);
            g_free (basename);
            ++count;
        }
    }
    /* update configuration file */
    for (i = 0; i < RECENT_FILES_NUM; ++i) {
        tstr = g_strdup_printf ("recent%d", i + 1);
        config_set_value (tstr, gui->recent_list[i]);
        g_free (tstr);
    }
}

void errorbuffer_set_text (gchar *message) {
    gtk_text_buffer_set_text (gui->errorbuff, message, -1);
}

void statusbar_set_message (gchar *message) {
    gtk_statusbar_push (GTK_STATUSBAR (gui->statusbar), gui->statusid, message);
    g_timeout_add_seconds (4, statusbar_del_message, NULL);
}

gboolean statusbar_del_message (void* user) {
    gtk_statusbar_pop (GTK_STATUSBAR (gui->statusbar), gui->statusid);
    return FALSE;
}

/**
 * @brief "changed" signal callback for editor->sourcebuffer
 * Automatically check whether to start timer if buffer changed.
 * Also set_modified for buffer
 */
void check_preview_timer (void) {
    gtk_text_buffer_set_modified (g_e_buffer, TRUE);
    gummi->latex->modified_since_compile = TRUE;
    gui_update_title ();

    motion_start_timer (gummi->motion);
}
