/**
 * @file    gui-main.c
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
#include "utils.h"
#include "template.h"

#include "compile/rubber.h"
#include "compile/latexmk.h"
#include "compile/texlive.h"


extern Gummi* gummi;
extern GummiGui* gui;

/* Many of the functions in this file are based on the excellent GTK+
 * tutorials written by Micah Carrick that can be found on:
 * http://www.micahcarrick.com/gtk-glade-tutorial-part-3.html */


/* These widgets are (de-)sensitised using the gui_has_tabs_sensitive function
 * for when we switch from 0 to 1 tabs and vice versa */
const gchar* insens_widgets_str[] = {
    "rightpanebox", "tool_save", "tool_bold", "tool_italic", "tool_unline",
    "tool_left", "tool_center", "tool_right", "menu_save", "menu_saveas",
    "menu_exportpdf", "import_tabs", "menu_close", "menu_export", 
    "menu_undo", "menu_redo", "menu_cut", "menu_copy", "menu_paste", 
    "menu_delete", "menu_selectall", "menu_preferences", "menu_find",
    "menu_prev", "menu_next", "menu_pdfcompile", "menu_compileopts",
    "menu_runmakeindex", "menu_runbibtex", "menu_docstat", "menu_spelling",
    "menu_snippets", "menu_edit", "menu_document", "menu_search",
    "menu_cleanup"
};


GummiGui* gui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GtkWidget* hpaned;
    gint i = 0, wx = 0, wy = 0, width = 0, height = 0;

    GummiGui* g = g_new0 (GummiGui, 1);
    
    g->builder = builder;

    g->mainwindow =
        GTK_WINDOW (gtk_builder_get_object (builder, "mainwindow"));
    g->toolbar = 
        GTK_WIDGET (gtk_builder_get_object (builder, "maintoolbar"));
    g->statusbar =
        GTK_STATUSBAR (gtk_builder_get_object (builder, "statusbar"));
    g->rightpane =
        GTK_VBOX (gtk_builder_get_object (builder, "rightpanebox"));
    g->previewoff = GTK_TOGGLE_TOOL_BUTTON (
            gtk_builder_get_object (builder, "tool_previewoff"));
    g->errorview =
        GTK_TEXT_VIEW (gtk_builder_get_object (builder, "errorview"));
    g->errorbuff =
        gtk_text_view_get_buffer (GTK_TEXT_VIEW (g->errorview));
    g->menu_spelling =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_spelling"));
    g->menu_snippets =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_snippets"));
    g->menu_toolbar =
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_toolbar"));
    g->menu_statusbar =
        GTK_CHECK_MENU_ITEM(gtk_builder_get_object (builder, "menu_statusbar"));
    g->menu_rightpane =
        GTK_CHECK_MENU_ITEM(gtk_builder_get_object (builder, "menu_rightpane"));
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

    g->docstatswindow =
        GTK_WIDGET (gtk_builder_get_object (builder, "docstatswindow"));
        
    g->menu_runbibtex =
        GTK_MENU_ITEM (gtk_builder_get_object (builder, "menu_runbibtex"));
    g->menu_runmakeindex =
        GTK_MENU_ITEM (gtk_builder_get_object (builder, "menu_runmakeindex"));
    g->bibcompile =
        GTK_WIDGET (gtk_builder_get_object (builder, "bibcompile"));

    g->insens_widget_size = sizeof(insens_widgets_str) / sizeof(gchar*);
    g->insens_widgets = g_new0(GtkWidget*, g->insens_widget_size);

    for (i = 0; i < g->insens_widget_size; ++i)
        g->insens_widgets[i] =
            GTK_WIDGET(gtk_builder_get_object (builder, insens_widgets_str[i]));

    g->menugui = menugui_init (builder);
    g->importgui = importgui_init (builder);
    g->previewgui = previewgui_init (builder);
    g->searchgui = searchgui_init (builder);
    g->prefsgui = prefsgui_init (g->mainwindow);
    g->snippetsgui = snippetsgui_init (g->mainwindow);
    g->tabmanagergui = tabmanagergui_init (builder);
    g->infoscreengui = infoscreengui_init (builder);
    g->projectgui = projectgui_init (builder);

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
    gtk_widget_modify_font (GTK_WIDGET (g->errorview), font_desc);
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
        gtk_widget_show (g->toolbar);
    } else {
        config_set_value ("toolbar", "False");
        gtk_check_menu_item_set_active (g->menu_toolbar, FALSE);
        gtk_widget_hide (g->toolbar);
    }

    if (config_get_value ("statusbar")) {
        gtk_check_menu_item_set_active (g->menu_statusbar, TRUE);
        gtk_widget_show (GTK_WIDGET (g->statusbar));
    } else {
        config_set_value ("statusbar", "False");
        gtk_check_menu_item_set_active (g->menu_statusbar, FALSE);
        gtk_widget_hide (GTK_WIDGET (g->statusbar));
    }

    if (config_get_value ("rightpane")) {
        gtk_check_menu_item_set_active (g->menu_rightpane, TRUE);
        gtk_widget_show (GTK_WIDGET (g->rightpane));
    } else {
        config_set_value ("compile_status", "False");
        gtk_toggle_tool_button_set_active (g->previewoff, FALSE);
        gtk_widget_hide (GTK_WIDGET (g->rightpane));
    }
    
    g->menu_autosync = 
        GTK_CHECK_MENU_ITEM (gtk_builder_get_object (builder, "menu_autosync"));
    
    if (latex_can_synctex() && config_get_value ("synctex")) {
        gtk_widget_set_sensitive (GTK_WIDGET (g->menu_autosync), TRUE);
        gboolean async = latex_use_synctex();
        gtk_check_menu_item_set_active (g->menu_autosync, (async? TRUE: FALSE));
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


G_MODULE_EXPORT
gboolean on_w32button_clicked (GtkWidget* widget, void* user) {
	#ifdef WIN32
		gtk_widget_hide (gui->w32window);
	#endif
	return TRUE;
}

#ifdef WIN32
gboolean w32popup_wait_event (void *builder) {

	const gchar* count = gtk_label_get_text (GTK_LABEL (gui->w32label));
	gint number = atoi (count) - 1;
	
	if (number == 0) {

		gtk_label_set_text (GTK_LABEL (gui->w32label), "");	
		gtk_widget_set_sensitive (gui->w32button, TRUE);
		return FALSE;
	}
	gtk_label_set_text (GTK_LABEL (gui->w32label), 
						g_strdup_printf("%d", number));
	return TRUE;
}
#endif

void gui_main (GtkBuilder* builder) {
    gtk_builder_connect_signals (builder, NULL);
    gtk_widget_show_all (GTK_WIDGET (gui->mainwindow));

	#ifdef WIN32
    gui->w32window = 
			GTK_WIDGET (gtk_builder_get_object (builder, "w32popup"));
	gui->w32button = 
			GTK_WIDGET (gtk_builder_get_object (builder, "w32button"));
	gui->w32label = 
			GTK_WIDGET (gtk_builder_get_object (builder, "w32label"));
    gtk_widget_show (gui->w32window);
    
    g_timeout_add_seconds (1, w32popup_wait_event, NULL);
    #endif

    gdk_threads_enter();
    gtk_main ();
    gdk_threads_leave();
}



G_MODULE_EXPORT
void on_menu_autosync_toggled (GtkCheckMenuItem *menu_autosync, void* user) {
    if (gtk_check_menu_item_get_active(menu_autosync)) {
        config_set_value("autosync", "True");
    } else {
        config_set_value("autosync", "False");
    }
}

G_MODULE_EXPORT
void on_tab_notebook_switch_page(GtkNotebook *notebook, GtkWidget *nbpage,
                                 int pagenr, void *data) {
    /* Kill typesetter command */
    motion_kill_typesetter(gummi->motion);

    /* set the active tab/editor pointers */
    tabmanager_set_active_tab (pagenr);

    /* update the title of the mainwindow */
    gui_set_filename_display (g_active_tab, TRUE, FALSE);
    
    /* clear the build log output window */
    gui_buildlog_set_text ("");
    
    previewgui_reset (gui->previewgui);
    slog (L_DEBUG, "Switched to environment at page %d\n", pagenr);
}

G_MODULE_EXPORT
void on_right_notebook_switch_page(GtkNotebook *notebook, GtkWidget *nbpage,
                                   int page, void *data) {
    if (page == 2) {
        if (gummi_project_active ()) {
            //projectgui_enable (gummi->project, gui->projectgui);
        }
        else {
            //projectgui_disable (gummi->project, gui->projectgui);
        }
    }
}

void gui_set_filename_display (GuTabContext* tc, gboolean title, gboolean label) {
    gchar* filetext = tabmanager_get_tabname (tc);
    
    if (label) tabmanagergui_update_label (tc->page, filetext);
    if (title) gui_set_window_title (tc->editor->filename, filetext);
}

void gui_set_window_title (const gchar* filename, const gchar* text) {
    gchar* dirname;
    gchar* title;

    if (filename != NULL) {
        dirname = g_strdup_printf("(%s)", g_path_get_dirname (filename));
        title = g_strdup_printf ("%s %s - %s", text, dirname,
                                                 PACKAGE_NAME);
    }
    else {
        title = g_strdup_printf ("%s - %s", text, PACKAGE_NAME);
    }
    gtk_window_set_title (gui->mainwindow, title);
    g_free (title);
}



void on_recovery_infobar_response (GtkInfoBar* bar, gint res, gpointer filename) {
    gchar* prev_workfile = iofunctions_get_swapfile (filename);
    
    if (res == GTK_RESPONSE_YES) {
        tabmanager_set_content (A_LOAD_OPT, filename, prev_workfile);
    }
    else { // NO
        tabmanager_set_content (A_LOAD, filename, NULL);
    }
    gui_recovery_mode_disable (bar);
}

void gui_recovery_mode_enable (GuTabContext* tab, const gchar* filename) {
    gchar* prev_workfile = iofunctions_get_swapfile (filename);

    slog (L_WARNING, "Swap file `%s' found.\n", prev_workfile);
    gchar* msg = g_strdup_printf (_("Swap file exists for %s, "
				"do you want to recover from it?"), filename);
    gtk_label_set_text (GTK_LABEL (tab->page->barlabel), msg);
    g_free (msg);
    
    gchar* data = g_strconcat (filename, NULL);
    tab->page->infosignal = 
        g_signal_connect (g_active_tab->page->infobar, "response",
        G_CALLBACK (on_recovery_infobar_response), (gpointer)data);
        
    gtk_widget_set_sensitive (GTK_WIDGET (tab->editor->view), FALSE);
    gtk_widget_show (tab->page->infobar);
}

void gui_recovery_mode_disable (GtkInfoBar *infobar) {
    gint id = g_active_tab->page->infosignal;
    g_signal_handler_disconnect (infobar, id);

    gtk_widget_hide (GTK_WIDGET(infobar));
    gtk_widget_set_sensitive (GTK_WIDGET (g_active_editor->view), TRUE);
}

void gui_open_file (const gchar* filename) {
    if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
        slog(L_G_ERROR, "Failed to open file '%s': No such file or "
                "directory\n", filename);
        return;
    }

    tabmanager_create_tab (A_LOAD, filename, NULL);
    if (!gtk_widget_get_sensitive (GTK_WIDGET (gui->rightpane))) {
        gui_set_hastabs_sensitive (TRUE);
	}
}

void gui_save_file (GuTabContext* tab, gboolean saveas) {
    gboolean new = FALSE;
    gchar* filename = NULL;
    gchar* pdfname = NULL;
    gchar* prev = NULL;
    gint ret = 0;

    if (saveas || !(filename = tab->editor->filename)) {
        if ((filename = get_save_filename (TYPE_LATEX))) {
            new = TRUE;
            if (!STR_EQU (filename + strlen (filename) -4, ".tex")) {
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
    
    gchar *text;
    GtkWidget* focus = NULL;

    focus = gtk_window_get_focus (gummi_get_gui ()->mainwindow);
    text = editor_grab_buffer (tab->editor);
    gtk_widget_grab_focus (focus);
    
    iofunctions_save_file (gummi->io, filename, text);

    if (config_get_value ("autoexport")) {
        pdfname = g_strdup (filename);
        pdfname[strlen (pdfname) -4] = 0;
        latex_export_pdffile (gummi->latex, tab->editor, pdfname, FALSE);
    }
    if (new) tabmanager_update_tab (filename);
    gui_set_filename_display (tab, TRUE, TRUE);
    gtk_widget_grab_focus (GTK_WIDGET (tab->editor->view));

cleanup:
    if (new) g_free (filename);
    g_free (pdfname);
}

void gui_set_hastabs_sensitive (gboolean enable) {
    gint i = 0;
    for (i = 0; i < gui->insens_widget_size; ++i)
        gtk_widget_set_sensitive (gui->insens_widgets[i], enable);
}

G_MODULE_EXPORT
void on_menu_bibupdate_activate (GtkWidget *widget, void * user) {
    biblio_compile_bibliography (gummi->biblio, g_active_editor, gummi->latex);
}

G_MODULE_EXPORT
gboolean on_docstats_close_clicked (GtkWidget* widget, void* user) {
    gtk_widget_hide (GTK_WIDGET (gui->docstatswindow));
    return TRUE;
}

G_MODULE_EXPORT
void on_tool_previewoff_toggled (GtkWidget *widget, void * user) {
    gboolean value =
        gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (widget));
    config_set_value ("compile_status", (!value)? "True": "False");
    if (value)
        previewgui_stop_preview (gui->previewgui);
    else
        previewgui_start_preview (gui->previewgui);
}

G_MODULE_EXPORT
void on_tool_textstyle_bold_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (g_active_editor, "tool_bold");
}

G_MODULE_EXPORT
void on_tool_textstyle_italic_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (g_active_editor, "tool_italic");
}

G_MODULE_EXPORT
void on_tool_textstyle_underline_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (g_active_editor, "tool_unline");
}

G_MODULE_EXPORT
void on_tool_textstyle_left_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (g_active_editor, "tool_left");
}

G_MODULE_EXPORT
void on_tool_textstyle_center_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (g_active_editor, "tool_center");
}

G_MODULE_EXPORT
void on_tool_textstyle_right_activate (GtkWidget* widget, void* user) {
    editor_set_selection_textstyle (g_active_editor, "tool_right");
}

G_MODULE_EXPORT
void on_button_template_add_clicked (GtkWidget* widget, void* user) {
    template_add_new_entry (gummi->templ);
}

G_MODULE_EXPORT
void on_button_template_remove_clicked (GtkWidget* widget, void* user) {
    template_remove_entry (gummi->templ);
}

G_MODULE_EXPORT
void on_button_template_open_clicked (GtkWidget* widget, void* user) {
    gchar* status = NULL;
    gchar* templ_name = template_get_selected_path (gummi->templ);

    if (templ_name) {
        /* add Loading message to status bar */
        status = g_strdup_printf (_("Loading template ..."));
        statusbar_set_message (status);
        g_free (status);

        tabmanager_create_tab (A_LOAD_OPT, NULL, templ_name);
        gtk_widget_hide (GTK_WIDGET (gummi->templ->templatewindow));
    }
    g_free(templ_name);
    
    if (!gtk_widget_get_sensitive (GTK_WIDGET (gui->rightpane)))
        gui_set_hastabs_sensitive (TRUE);
}

G_MODULE_EXPORT
void on_button_template_close_clicked (GtkWidget* widget, void* user) {
    gtk_widget_set_sensitive (GTK_WIDGET (gummi->templ->template_add), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (gummi->templ->template_remove), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (gummi->templ->template_open), TRUE);
    gtk_widget_hide (GTK_WIDGET (gummi->templ->templatewindow));
}

G_MODULE_EXPORT
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
        text = editor_grab_buffer (g_active_editor);
        template_create_file (gummi->templ, filenm, text);
    }
    g_free (text);
    g_free (filepath);
}

G_MODULE_EXPORT 
void on_template_cursor_changed (GtkTreeView *tree, gpointer data) {
    if (!gtk_tree_view_get_selection (tree) == 0) {
        gtk_widget_set_sensitive (gummi->templ->template_open, TRUE);
    }
}
    

G_MODULE_EXPORT
void on_bibcolumn_clicked (GtkWidget* widget, void* user) {
    gint id = gtk_tree_view_column_get_sort_column_id
        (GTK_TREE_VIEW_COLUMN (widget));
    gtk_tree_view_column_set_sort_column_id
        (GTK_TREE_VIEW_COLUMN (widget), id);
}

G_MODULE_EXPORT
void on_button_biblio_compile_clicked (GtkWidget* widget, void* user) {
    gummi->biblio->progressval = 0.0;
    
    gtk_widget_set_sensitive (widget, FALSE);
    g_timeout_add (10, on_bibprogressbar_update, NULL);

    if (biblio_compile_bibliography (gummi->biblio, g_active_editor,
                gummi->latex)) {
        statusbar_set_message (_("Compiling bibliography file..."));
        gtk_progress_bar_set_text (gummi->biblio->progressbar,
                _("Bibliography compiled without errors"));
        motion_force_compile (gummi->motion);
    } else {
        statusbar_set_message 
        (_("Error compiling bibliography file or none detected..."));
        gtk_progress_bar_set_text (gummi->biblio->progressbar,
                _("Error compiling bibliography file"));
    }
}

G_MODULE_EXPORT
void on_button_biblio_detect_clicked (GtkWidget* widget, void* user) {
    gchar* text = 0;
    gchar* str = 0;
    gchar* basename = 0;
    GError* err = NULL;
    gint number = 0;

    gummi->biblio->progressval = 0.0;
    g_timeout_add (2, on_bibprogressbar_update, NULL);
    gtk_list_store_clear (gummi->biblio->list_biblios);

    if (biblio_detect_bibliography (gummi->biblio, g_active_editor)) {
        editor_insert_bib (g_active_editor, g_active_editor->bibfile);
        if (!g_file_get_contents(g_active_editor->bibfile, &text, NULL, &err)) {
            slog (L_G_ERROR, "g_file_get_contents (): %s\n", err->message);
            g_error_free (err);
            return;
        }

        gtk_widget_set_sensitive
                    (GTK_WIDGET(gummi->biblio->list_filter), TRUE);

        number = biblio_parse_entries (gummi->biblio, text);
        basename = g_path_get_basename (g_active_editor->bibfile);
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

G_MODULE_EXPORT
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

G_MODULE_EXPORT
void on_biblio_filter_changed (GtkWidget* widget, void* user) {
    GtkTreeModel *filter;

    const gchar *entry = gtk_entry_get_text(GTK_ENTRY(widget));
    filter = gtk_tree_model_filter_new (
            GTK_TREE_MODEL(gummi->biblio->list_biblios),NULL);
    gtk_tree_model_filter_set_visible_func(
            GTK_TREE_MODEL_FILTER(filter), visible_func, (gpointer)entry, NULL);
    gtk_tree_view_set_model (
            GTK_TREE_VIEW( gummi->biblio->biblio_treeview ),filter);
    g_object_unref (G_OBJECT(filter));
}

void typesetter_setup (void) {
    // change the pref gui options on changing typesetter:
    gboolean status = texlive_active();
    gtk_widget_set_sensitive (GTK_WIDGET (gui->menu_runbibtex), status);
    gtk_widget_set_sensitive (GTK_WIDGET (gui->menu_runmakeindex), status);
    gtk_widget_set_sensitive (GTK_WIDGET (gui->prefsgui->opt_shellescape),
                              status);
                              
    gboolean texormk = (texlive_active() || latexmk_active());
    
    
    
    if (config_get_value("synctex") && texormk) {
        gtk_toggle_button_set_active (gui->prefsgui->opt_synctex, TRUE);
    }
    else {
        gtk_toggle_button_set_active (gui->prefsgui->opt_synctex, FALSE);
    }
    gtk_widget_set_sensitive (GTK_WIDGET (gui->prefsgui->opt_synctex), texormk);
    
    
    slog (L_INFO, "Typesetter %s configured.\n",config_get_value("typesetter"));
}

gboolean on_bibprogressbar_update (void* user) {
    gtk_adjustment_set_value
        (gummi->biblio->progressmon, gummi->biblio->progressval);
    gummi->biblio->progressval += 1.0;
    if (gummi->biblio->progressval > 60) {
        gtk_widget_set_sensitive (gui->bibcompile, TRUE);
        return FALSE;
    }
    return TRUE;
}

gint check_for_save (GuEditor* editor) {
    gint ret = 0;

    if (editor && editor_buffer_changed (editor))
        ret = utils_yes_no_dialog (
                _("Do you want to save the changes you have made?"));
    return ret;
}

gchar* get_open_filename (GuFilterType type) {
    GtkFileChooser* chooser = NULL;
    gchar* active_cwd = NULL;
    gchar* filename = NULL;


    /* check if both active_editor and filename are defined. when zero tabs
     * are open, the g_active_editor object is destroyed causing a segfault
     * on the (filename != null) check. */
    if (g_active_editor != NULL && g_active_editor->filename != NULL) {
       active_cwd = g_path_get_dirname(g_active_editor->filename);
    }

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
    if (active_cwd)
        gtk_file_chooser_set_current_folder (chooser, active_cwd);
    else
        gtk_file_chooser_set_current_folder (chooser, g_get_home_dir ());

    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

    if (filename) {
        g_free (active_cwd);
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
        "shouldn't happen",
        _("Create project")
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

    if (g_active_editor->filename) {
        gchar* dirname = g_path_get_dirname (g_active_editor->filename);
        gchar* basename = g_path_get_basename (g_active_editor->filename);

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
            gtk_file_filter_set_name (filter, _("LaTeX files"));
            gtk_file_filter_add_pattern (filter, "*.tex");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            filter = gtk_file_filter_new ();
            gtk_file_filter_set_name (filter, _("Text files"));
            gtk_file_filter_add_mime_type (filter, "text/plain");
            gtk_file_chooser_add_filter (dialog, filter);
            break;

        case TYPE_PDF:
            gtk_file_filter_set_name (filter, _(("PDF files")));
            gtk_file_filter_add_pattern (filter, "*.pdf");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            break;

        case TYPE_IMAGE: 
            /* Only \insertgraphics uses this section now. Make sure 
             * the patterns & mimes are correct before assigning it
			 * for other functions */
            gtk_file_filter_set_name (filter, _("Supported Image files"));
            
            /* Pdflatex supports different formats than pure latex */
            if (latex_method_active ("texpdf")) {
				gtk_file_filter_add_pattern (filter, "*.jpg");
				gtk_file_filter_add_pattern (filter, "*.jpeg");
				gtk_file_filter_add_pattern (filter, "*.png");
				gtk_file_filter_add_pattern (filter, "*.pdf");
			}
			gtk_file_filter_add_pattern (filter, "*.eps");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            break;

        case TYPE_BIBLIO:
            gtk_file_filter_set_name (filter, _("Bibtex files"));
            gtk_file_filter_add_pattern (filter, "*.bib");
            gtk_file_chooser_add_filter (dialog, filter);
            gtk_file_chooser_set_filter (dialog, filter);
            break;
        case TYPE_PROJECT:
            gtk_file_filter_set_name (filter, _("Gummi project files"));
            gtk_file_filter_add_pattern (filter, "*.gummi");
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
        if (STR_EQU (filename, gui->recent_list[i]))
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
        if (!STR_EQU (gui->recent_list[i], "__NULL__")) {
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
    /* update recent files */
    for (i = 0; i < RECENT_FILES_NUM; ++i) {
        tstr = g_strdup_printf ("recent%d", i + 1);
        config_set_value (tstr, gui->recent_list[i]);
        g_free (tstr);
    }
}

void gui_buildlog_set_text (const gchar *message) {
    if (message) {
        GtkTextIter iter;
        gtk_text_buffer_set_text (gui->errorbuff, message, -1);
        gtk_text_buffer_get_end_iter (gui->errorbuff, &iter);
		// The following lines are commented out, as they seem to cause 
		// Bug #252.
        //GtkTextMark *mark = gtk_text_buffer_create_mark(gui->errorbuff, NULL, &iter, FALSE);
        //gtk_text_view_scroll_to_mark (gui->errorview, mark, 0.25, FALSE, 0, 0);
        //gtk_text_view_scroll_to_iter (gui->errorview, &iter, 0.25, FALSE, 0, 0);
    }
}

void statusbar_set_message (const gchar *message) {
    gtk_statusbar_push (GTK_STATUSBAR (gui->statusbar), gui->statusid, message);
    g_timeout_add_seconds (4, statusbar_del_message, NULL);
}

gboolean statusbar_del_message (void* user) {
    gtk_statusbar_pop (GTK_STATUSBAR (gui->statusbar), gui->statusid);
    return FALSE;
}

/**
 * @brief "changed" signal callback for editor->buffer
 * Automatically check whether to start timer if buffer changed.
 * Also set_modified for buffer
 */
void check_preview_timer (void) {
    g_return_if_fail (g_active_tab != NULL);
    
    gtk_text_buffer_set_modified (g_e_buffer, TRUE);
    gummi->latex->modified_since_compile = TRUE;
    
    gui_set_filename_display (g_active_tab, TRUE, TRUE);

    motion_start_timer (gummi->motion);
}
