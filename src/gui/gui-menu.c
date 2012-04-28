/**
 * @file   gui-menu.c
 * @brief
 *
 * Copyright (C) 2009-2012 Gummi-Dev Team <alexvandermey@gmail.com>
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
#include "constants.h"
#include "editor.h"
#include "environment.h"
#include "external.h"
#include "gui-main.h"
#include "motion.h"
#include "project.h"
#include "update.h"

extern Gummi* gummi;
extern GummiGui* gui;

/* TODO: split the gui struct up into smaller pieces and try to remove all
 * non-gui functions and the "extern gummi" from this file */

GuMenuGui* menugui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuMenuGui* m = g_new0 (GuMenuGui, 1);
    
    m->menu_projcreate = GTK_MENU_ITEM(
                        gtk_builder_get_object (builder, "menu_projcreate"));
    m->menu_projopen = GTK_MENU_ITEM(
                        gtk_builder_get_object (builder, "menu_projopen"));
    m->menu_projclose = GTK_MENU_ITEM(
                        gtk_builder_get_object (builder, "menu_projclose"));
    
    #ifdef WIN32
    // Please do NOT enable for nix, it has no place on a free OS ;)
    GtkWidget* donate = 
		gtk_image_menu_item_new_with_label ("Support this Project");
    gtk_image_menu_item_set_image 
        (GTK_IMAGE_MENU_ITEM(donate), 
         GTK_WIDGET(gtk_image_new_from_stock(
         GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_MENU)));
    GtkWidget* helpmenu = 
		GTK_WIDGET (gtk_builder_get_object (builder, "menu11"));
    gtk_menu_prepend (GTK_MENU (helpmenu), donate);
    gtk_signal_connect_object (GTK_OBJECT (donate), "activate",
                               GTK_SIGNAL_FUNC (on_menu_donate_activate),
                               NULL);
    #endif
    
    /* TODO: There has to be a better way than this.. (bug 246)
    GtkSettings *settings = gtk_settings_get_default();
    gchar *iconsizes;
    g_object_get(settings, "gtk-icon-sizes", &iconsizes, NULL);
    if (iconsizes != NULL) {
        printf("%s\n", iconsizes);
    }*/ 

    #ifdef WIN32
        // The 2 non-stock menu items have their pixel size values set 
        // to the default 16x16 in GLADE. The normal icon-size value is 
        // normally set by the GTK theme in gtkrc. For themes using
        // non-default icon sizes or Windows, this 16x16 value will be
        // wrong. This code sets it to match the gtkrc file that we 
        // supply with the Windows builds:
        GtkWidget* export = gtk_image_menu_item_get_image (
        GTK_IMAGE_MENU_ITEM (gtk_builder_get_object (builder, "menu_export")));
        GtkWidget* update = gtk_image_menu_item_get_image (
        GTK_IMAGE_MENU_ITEM (gtk_builder_get_object (builder, "menu_update")));
        gtk_image_set_pixel_size (GTK_IMAGE(export), 13);
        gtk_image_set_pixel_size (GTK_IMAGE(update), 13);
    #endif
 

    
    
    return m;
}

#ifdef WIN32
G_MODULE_EXPORT
void on_menu_donate_activate (GtkWidget* widget, void* user) {
	//TODO: enhance
	GtkWidget* dialog;
    dialog = gtk_message_dialog_new (gui->mainwindow, 
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Gummi is released as free open-source software under the MIT "
        "license. Development is carried out and continued as a spare "
        "time activity. \nIf you are interested in supporting this "
        "project with a donation, please visit our official website. "
        "Thank you!");
    gtk_window_set_title (GTK_WINDOW (dialog), "Support this project");
    gtk_dialog_run (GTK_DIALOG (dialog));      
    gtk_widget_destroy (dialog);
}
#endif

/*******************************************************************************
 * FILE MENU                                                                   *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_new_activate (GtkWidget *widget, void* user) {
    if (!gtk_widget_get_sensitive (GTK_WIDGET (gui->rightpane)))
        gui_set_hastabs_sensitive (TRUE);
    tabmanager_create_tab (A_NONE, NULL, NULL);
}

G_MODULE_EXPORT
void on_menu_template_activate (GtkWidget *widget, void *user) {
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
    gui_save_file (g_active_tab, FALSE);
}

G_MODULE_EXPORT
void on_menu_saveas_activate (GtkWidget *widget, void* user) {
    gui_save_file (g_active_tab, TRUE);
}

G_MODULE_EXPORT
void on_menu_export_activate (GtkWidget *widget, void *user) {
    gchar* filename = NULL;

    filename = get_save_filename (TYPE_PDF);
    if (filename)
        latex_export_pdffile (gummi->latex, g_active_editor, filename, TRUE);
    g_free (filename);
}

G_MODULE_EXPORT
void on_menu_recent_activate (GtkWidget *widget, void *user) {
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
    GuTabContext* tab = NULL;
    
    tab = (user)? GU_TAB_CONTEXT (user): g_active_tab;
    
    gint ret = check_for_save (tab->editor);
    
    if (GTK_RESPONSE_YES == ret)
        gui_save_file (tab, FALSE);
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;
    
    /* Kill typesetter command */
    motion_kill_typesetter(gummi->motion);

    if (!tabmanager_remove_tab (tab)) {
        motion_start_errormode (gummi->motion, "");
        gui_set_hastabs_sensitive (FALSE);
    } else {
        gui_set_filename_display (g_active_tab, TRUE, FALSE);
    }
}

G_MODULE_EXPORT
gboolean on_menu_quit_activate (void) {
    gint wx = 0, wy = 0, width = 0, height = 0;
    gchar buf[16];
    int i = 0;
    gint length = g_list_length (gummi->tabmanager->tabs);

    /* Stop compile thread */
    if (length > 0) motion_stop_compile_thread (gummi->motion);

    for(i = 0; i < length; i++){
        gtk_notebook_set_current_page(gui->tabmanagergui->notebook, i);
        tabmanager_set_active_tab (i);

        gint ret = check_for_save (g_active_editor);
        if (GTK_RESPONSE_YES == ret)
            gui_save_file (g_active_tab, FALSE);
        else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
            return TRUE;
    }

    gtk_window_get_size (gui->mainwindow, &width, &height);
    gtk_window_get_position (gui->mainwindow, &wx, &wy);

    config_begin();
    config_set_value ("mainwindow_x", g_ascii_dtostr (buf, 16, (double)wx));
    config_set_value ("mainwindow_y", g_ascii_dtostr (buf, 16, (double)wy));
    config_set_value ("mainwindow_w", g_ascii_dtostr (buf, 16, (double)width));
    config_set_value ("mainwindow_h", g_ascii_dtostr (buf, 16, (double)height));
    config_commit();

    gtk_main_quit ();

    for(i = 0; i < length; i++)
        editor_destroy (GU_TAB_CONTEXT (g_list_nth_data
                    (gummi->tabmanager->tabs, i))->editor);

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
void on_menu_delete_activate (GtkWidget *widget, void *user) {
    gtk_text_buffer_delete_selection (g_e_buffer, FALSE, TRUE);
}

G_MODULE_EXPORT
void on_menu_selectall_activate (GtkWidget *widget, void *user) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (g_e_buffer, &start, &end);
    gtk_text_buffer_select_range (g_e_buffer, &start, &end);
}

G_MODULE_EXPORT
void on_menu_preferences_activate (GtkWidget *widget, void *user) {
    prefsgui_main (gui->prefsgui, 0);
}

/*******************************************************************************
 * VIEW MENU                                                                   *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_statusbar_toggled (GtkWidget *widget, void *user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        gtk_widget_show (GTK_WIDGET (gui->statusbar));
        config_set_value ("statusbar", "True");
    } else {
        gtk_widget_hide (GTK_WIDGET (gui->statusbar));
        config_set_value ("statusbar", "False");
    }
}

G_MODULE_EXPORT
void on_menu_toolbar_toggled (GtkWidget *widget, void *user) {
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
        gtk_widget_show (gui->toolbar);
        config_set_value ("toolbar", "True");
    } else {
        gtk_widget_hide (gui->toolbar);
        config_set_value ("toolbar", "False");
    }
}

G_MODULE_EXPORT
void on_menu_rightpane_toggled (GtkWidget *widget, void *user) {
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
void on_menu_fullscreen_toggled (GtkWidget *widget, void *user) {
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
void on_menu_findnext_activate (GtkWidget *widget, void *user) {
    editor_jumpto_search_result (g_active_editor, 1);
}

G_MODULE_EXPORT
void on_menu_findprev_activate (GtkWidget *widget, void *user) {
    editor_jumpto_search_result (g_active_editor, -1);
}

/*******************************************************************************
 * DOCUMENT MENU                                                               *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_pdfcompile_activate (GtkWidget *widget, void* user) {
    gummi->latex->modified_since_compile = TRUE;
    motion_do_compile (gummi->motion);
}

G_MODULE_EXPORT
void on_menu_compileopts_activate (GtkWidget* widget, void* user) {
    prefsgui_main(gui->prefsgui, 4);
}

G_MODULE_EXPORT
void on_menu_cleanup_activate (GtkWidget *widget, void *user) {
    int result = latex_remove_auxfile (g_active_editor);

    if (result == 0) {
        statusbar_set_message (_("Succesfully removed build files.."));
    }
    else {
        statusbar_set_message (_("Error removing build files.."));
    }
}

G_MODULE_EXPORT
void on_menu_runmakeindex_activate (GtkWidget *widget, void *user) {
    if (latex_run_makeindex (g_active_editor)) {
        statusbar_set_message (_("Running Makeindex.."));
    }
    else {
        statusbar_set_message (_("Error running Makeindex.."));
    }
    motion_force_compile (gummi->motion);
}

G_MODULE_EXPORT
void on_menu_runbibtex_activate (GtkWidget *widget, void *user) {
    on_button_biblio_compile_clicked (widget, user);
}

G_MODULE_EXPORT
void on_menu_docstat_activate (GtkWidget *widget, void *user) {
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
    if (external_exists ("texcount")) {
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
        Tuple2 result = utils_popen_r (cmd, NULL);

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
    

    gtk_label_set_text (GTK_LABEL (gtk_builder_get_object (gui->builder, 
                    "stats_filename")), tabmanagergui_get_labeltext (g_active_tab->page));
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
void on_menu_spelling_toggled (GtkWidget *widget, void *user) {
#ifdef USE_GTKSPELL
    GList *editors;
    GuEditor* ec;
    int ectotal, i;
    
    gboolean activate = gtk_check_menu_item_get_active 
                        (GTK_CHECK_MENU_ITEM (widget));
    
    editors = gummi_get_all_editors ();
    ectotal = g_list_length (editors);
    
    for (i=0; i<ectotal; i++) {
        ec = g_list_nth_data (editors, i);
        if (activate) {
            editor_activate_spellchecking (ec, TRUE);
        }
        else { 
            editor_activate_spellchecking (ec, FALSE);
        }
    }
    if (activate) config_set_value ("spelling", "True");
    else { config_set_value ("spelling", "False"); }
            
#endif
}

G_MODULE_EXPORT
void on_menu_snippets_toggled (GtkWidget *widget, void *user) {
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
    // TODO: perhaps use buffer to run pre compile check */
    
    if (g_active_editor == NULL) {
		gtk_widget_set_tooltip_text (GTK_WIDGET 
					 (gui->menugui->menu_projcreate),
					_("This function requires an active document"));
		return;
	}
    
    if (!gummi_project_active()) {
        gtk_widget_set_sensitive (GTK_WIDGET
                                 (gui->menugui->menu_projopen), TRUE);
        // TODO we should probably have functions for calls like this:
        if (g_active_editor->filename != NULL) {
            gtk_widget_set_sensitive (GTK_WIDGET
                                 (gui->menugui->menu_projcreate), TRUE);                    
        }
        else {
			gtk_widget_set_tooltip_text (GTK_WIDGET 
					 (gui->menugui->menu_projcreate),
					_("This function requires the current\n"
					  "active document to be saved. "));
		}
    }
    else {
        gtk_widget_set_sensitive (GTK_WIDGET
                                 (gui->menugui->menu_projclose), TRUE);
    }
}

G_MODULE_EXPORT
void on_menu_project_deselect (GtkWidget *widget, void *user) {
     gtk_widget_set_sensitive (GTK_WIDGET(
                               gui->menugui->menu_projcreate), FALSE);
     gtk_widget_set_sensitive (GTK_WIDGET(
                               gui->menugui->menu_projopen), FALSE);
     gtk_widget_set_sensitive (GTK_WIDGET(
                               gui->menugui->menu_projclose), FALSE);
}

G_MODULE_EXPORT
void on_menu_projcreate_activate (GtkWidget *widget, void *user) {

    gchar* filename = get_save_filename (TYPE_PROJECT);
    if (!filename) return;
    
    if (project_create_new (filename)) {
        projectgui_enable (gummi->project, gui->projectgui);
        projectgui_list_projfiles (gummi->project->projfile);
    }
}

G_MODULE_EXPORT
void on_menu_projopen_activate (GtkWidget *widget, void *user) {
    
    gchar* filename = get_open_filename (TYPE_PROJECT);
    if (!filename) return;

    if (project_open_existing (filename)) {
        statusbar_set_message (g_strdup_printf("Loading project %s", filename));
        projectgui_enable (gummi->project, gui->projectgui);
        projectgui_list_projfiles (gummi->project->projfile);
    }
    else {
        statusbar_set_message (g_strdup_printf("An error ocurred while "
                                               "loading project %s", filename));
    }
    g_free (filename);
}

G_MODULE_EXPORT
void on_menu_projclose_activate (GtkWidget *widget, void *user) {
    
    if (!gummi->project->projfile) return;
    
    if (project_close ()) {
        projectgui_disable (gummi->project, gui->projectgui);
    }
    
    
}

/*******************************************************************************
 * HELP MENU                                                                   *
 ******************************************************************************/

G_MODULE_EXPORT
void on_menu_guide_activate (GtkWidget *widget, void *user) {
    GError *error = NULL;

    gtk_show_uri (gdk_screen_get_default(), 
                  C_GUMMIGUIDE, GDK_CURRENT_TIME, &error);
    
    if (error) {
        slog (L_ERROR, "Can't open user guide: %s\n", error->message);
    }
}

G_MODULE_EXPORT
void on_menu_update_activate (GtkWidget *widget, void *user) {
    #ifdef WIN32
        slog (L_G_INFO, "To be implemented for win32..\n");
    #else
        gboolean ret = updatecheck (gui->mainwindow);
        if (!ret)
            slog (L_G_ERROR, "Update check failed!\n");
    #endif
}

G_MODULE_EXPORT
void on_menu_about_activate (GtkWidget *widget, void *user) {
    GError* err = NULL;
    gchar* icon_file = g_build_filename (DATADIR, "icons", "gummi.png", NULL);
    GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_size (icon_file, 80, 80, &err);
    g_free (icon_file);

    const gchar* authors[] = { "Alexander van der Mey\n"
        "<alexvandermey@gmail.com>",
        "Wei-Ning Huang\n"
        "<aitjcize@gmail.com>",
        "Dion Timmermann",
        "<dion.timmermann@tu-harburg.de>", 
        "Robert Schroll",
        "<rschroll@gmail.com>\n",
        "Former contributors:",
        "Thomas van der Burgt",
        "Cameron Grout", NULL };

    const gchar* translators =
        "**  Visit our website for instructions on  **\n"
        "**  contributing or updating a translation **\n"
        "\n"
        "Arabic: Hamad Mohammad\n"
        "Brazilian-Portugese: Fernando Cruz & Alexandre Guimarães\n"
        "Catalan: Marc Vinyals\n"
        "Chinese (Simplified): Mathlab pass\n"
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
        "Swedish: Kess Vargavind\n"
        "Spanish: Carlos Salas Contreras\n";
        
    const gchar* documenters[] = {"Guy Edwards", NULL};

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
    gtk_about_dialog_set_documenters (dialog, documenters);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));
}








