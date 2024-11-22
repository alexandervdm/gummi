/**
 * @file   gui-prefs.c
 * @brief
 *
 * Copyright (C) 2009 Gummi Developers
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

#include "gui-prefs.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "constants.h"
#include "configfile.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "latex.h"
#include "utils.h"

#include "compile/rubber.h"
#include "compile/tectonic.h"
#include "compile/latexmk.h"
#include "compile/texlive.h"


/* TODO: needs mayor cleanup */

extern GummiGui* gui;
extern Gummi* gummi;

static void set_all_tab_settings (GuPrefsGui* prefs);
static void set_tab_view_settings (GuPrefsGui* prefs);
static void set_tab_editor_settings (GuPrefsGui* prefs);
static void set_tab_fontcolor_settings (GuPrefsGui* prefs);
static void set_tab_defaulttext_settings (GuPrefsGui* prefs);
static void set_tab_compilation_settings (GuPrefsGui* prefs);
static void set_tab_preview_settings (GuPrefsGui* prefs);
static void set_tab_miscellaneous_settings (GuPrefsGui* prefs);


GuPrefsGui* prefsgui_init (GtkWindow* mainwindow) {
    GuPrefsGui* p = g_new0 (GuPrefsGui, 1);
    GtkBuilder* builder = gtk_builder_new ();
    gchar* ui = g_build_filename (GUMMI_DATA, "ui", "prefs.glade", NULL);
    gtk_builder_add_from_file (builder, ui, NULL);
    gtk_builder_set_translation_domain (builder, C_PACKAGE);
    g_free (ui);

    p->prefwindow =
        GTK_WIDGET (gtk_builder_get_object (builder, "prefwindow"));
    p->notebook =
        GTK_NOTEBOOK (gtk_builder_get_object (builder, "notebook1"));
    p->textwrap_button =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "textwrapping"));
    p->wordwrap_button =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "wordwrapping"));
    p->line_numbers =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "line_numbers"));
    p->highlighting =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "highlighting"));
    p->tabwidth =
        GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "tabwidth"));
    p->spaces_instof_tabs =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "spaces_instof_tabs"));
    p->autoindentation =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "autoindentation"));
    p->autosaving =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "autosaving"));
    p->compile_status =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "compile_status"));
    p->autosave_timer =
        GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "autosave_timer"));
    p->combo_languages =
        GTK_COMBO_BOX_TEXT (gtk_builder_get_object (builder, "combo_languages"));
    p->styleschemes_treeview =
        GTK_TREE_VIEW(gtk_builder_get_object(builder, "styleschemes_treeview"));
    p->list_styleschemes =
        GTK_LIST_STORE (gtk_builder_get_object (builder, "list_styleschemes"));
    p->default_text =
        GTK_TEXT_VIEW (gtk_builder_get_object (builder, "default_text"));
    p->default_buffer =
        gtk_text_view_get_buffer (p->default_text);
    p->editor_font =
        GTK_FONT_BUTTON (gtk_builder_get_object (builder, "editor_font"));
    p->compile_scheme =
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "combo_compilescheme"));
    p->compile_timer =
        GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "compile_timer"));
    p->autoexport =
        GTK_CHECK_BUTTON (gtk_builder_get_object (builder, "auto_export"));

    p->typ_pdflatex =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "typ_pdflatex"));
    p->typ_xelatex =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "typ_xelatex"));
    p->typ_lualatex =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "typ_lualatex"));
    p->typ_rubber =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "typ_rubber"));
    p->typ_tectonic =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "typ_tectonic"));
    p->typ_latexmk =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "typ_latexmk"));

    p->method_texpdf =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "method_texpdf"));
    p->method_texdvipdf =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "method_texdvipdf"));
    p->method_texdvipspdf =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "method_texdvipspdf"));

    p->opt_shellescape =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "opt_shellescape"));
    p->opt_synctex =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "opt_synctex"));

    p->combo_zoom_modes =
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "combo_zoom_modes"));
    p->list_zoom_modes =
        GTK_LIST_STORE (gtk_builder_get_object (builder, "list_zoom_modes"));
    p->combo_animated_scroll =
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "combo_animated_scroll"));
    p->spin_cache_size =
        GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "spin_cache_size"));

    gtk_window_set_transient_for (GTK_WINDOW (p->prefwindow), mainwindow);

    // list available languages
    if (g_file_test (
        g_find_program_in_path("enchant-lsmod-2"), G_FILE_TEST_EXISTS)) {

        Tuple2 pret = utils_popen_r ("enchant-lsmod-2 -list-dicts", NULL);
        if (pret.second != NULL) {
            gchar** output = g_strsplit((gchar*)pret.second, "\n", BUFSIZ);
            gchar** elems = NULL;
            int i;

            for(i = 0; output[i] != NULL; i++) {
                elems = g_strsplit (output[i], " ", BUFSIZ);
                if (elems[0] != NULL) {
                    gtk_combo_box_text_append_text (p->combo_languages, elems[0]);
                }
            }
        g_strfreev(output);
        g_strfreev(elems);
        }
        gtk_combo_box_set_active (GTK_COMBO_BOX(p->combo_languages), 0);
        g_free ((gchar*)pret.second);
    }

    GList* schemes = editor_list_style_scheme_sorted ();
    GList* schemes_iter = schemes;
    gchar* desc = NULL;
    GtkTreeIter iter;

    while (schemes_iter) {
        desc = g_markup_printf_escaped ("<b>%s</b> - %s",
                gtk_source_style_scheme_get_name (schemes_iter->data),
                gtk_source_style_scheme_get_description (schemes_iter->data));
        gtk_list_store_append (p->list_styleschemes, &iter);
        gtk_list_store_set (p->list_styleschemes, &iter,
                0, desc,
                1, gtk_source_style_scheme_get_id (schemes_iter->data), -1);
        schemes_iter = g_list_next (schemes_iter);
        g_free (desc);
    }

    g_list_free (schemes);

    gtk_builder_connect_signals (builder, NULL);

    return p;
}

void prefsgui_main (GuPrefsGui* prefs, int page) {

    gtk_notebook_set_current_page(prefs->notebook, page);

    set_all_tab_settings (prefs);
    gtk_widget_show_all (GTK_WIDGET (prefs->prefwindow));
}

static void set_all_tab_settings (GuPrefsGui* prefs) {
    set_tab_view_settings (prefs);
    set_tab_editor_settings (prefs);
    set_tab_fontcolor_settings (prefs);
    set_tab_defaulttext_settings (prefs);
    set_tab_compilation_settings (prefs);
    set_tab_preview_settings (prefs);
    set_tab_miscellaneous_settings (prefs);
}

static void set_tab_view_settings (GuPrefsGui* prefs) {
    gboolean textwrap_enabled = config_get_boolean ("Editor", "textwrapping");

    if (textwrap_enabled) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(prefs->textwrap_button),
                                      textwrap_enabled);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(prefs->wordwrap_button),
                                      config_get_boolean ("Editor", "wordwrapping"));
    } else
        gtk_widget_set_sensitive (GTK_WIDGET (prefs->wordwrap_button), FALSE);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->line_numbers),
                                  config_get_boolean ("Editor", "line_numbers"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->highlighting),
                                  config_get_boolean ("Editor", "highlighting"));

}

static void set_tab_editor_settings (GuPrefsGui* prefs) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->autoindentation),
                                  config_get_boolean ("Editor", "autoindentation"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->spaces_instof_tabs),
                                  config_get_boolean ("Editor", "spaces_instof_tabs"));
    gtk_spin_button_set_value (prefs->tabwidth,
                               config_get_integer ("Editor", "tabwidth"));
    gtk_spin_button_set_value (prefs->autosave_timer,
                               config_get_integer ("File", "autosave_timer"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->autosaving),
                                  config_get_boolean ("File", "autosaving"));
    if (!config_get_boolean ("File", "autosaving"))
        gtk_widget_set_sensitive (GTK_WIDGET (prefs->autosave_timer), FALSE);
}

static void set_tab_fontcolor_settings (GuPrefsGui* prefs) {
    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (prefs->editor_font),
                               config_get_string ("Editor", "font_str"));
    prefsgui_apply_style_scheme(prefs);

    // set default font on all tabs
    GList* tab = gummi->tabmanager->tabs;
    while (tab) {
        editor_set_font (GU_TAB_CONTEXT (tab->data)->editor,
                         config_get_string ("Editor", "font_css"));
        tab = g_list_next (tab);
    }
}

static void set_tab_defaulttext_settings (GuPrefsGui* prefs) {

    gchar* text = NULL;

    if (!g_file_get_contents (C_WELCOMETEXT, &text, NULL, NULL)) {
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->default_text), FALSE);
        return;
    }
    gtk_text_buffer_set_text (prefs->default_buffer, text, -1);
    gtk_text_buffer_set_modified (prefs->default_buffer, FALSE);
    g_free (text);
}

static void set_tab_compilation_settings (GuPrefsGui* prefs) {
    /* Setting available typesetters and the active one */
    /* TODO: iterate the available typesetter list and gtk_builder the objects
     * maybe.. or not.. */


    if (pdflatex_detected()) {
        if (pdflatex_active())
            gtk_toggle_button_set_active (prefs->typ_pdflatex, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->typ_pdflatex), TRUE);
        gtk_widget_set_tooltip_text (GTK_WIDGET(prefs->typ_pdflatex), "");
    }

    if (xelatex_detected()) {
        if (xelatex_active())
            gtk_toggle_button_set_active (prefs->typ_xelatex, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->typ_xelatex), TRUE);
        gtk_widget_set_tooltip_text (GTK_WIDGET(prefs->typ_xelatex), "");
    }

    if (lualatex_detected()) {
        if (lualatex_active())
            gtk_toggle_button_set_active (prefs->typ_lualatex, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->typ_lualatex), TRUE);
        gtk_widget_set_tooltip_text (GTK_WIDGET(prefs->typ_lualatex), "");
    }

    if (rubber_detected()) {
        if (rubber_active())
            gtk_toggle_button_set_active (prefs->typ_rubber, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->typ_rubber), TRUE);
        gtk_widget_set_tooltip_text (GTK_WIDGET(prefs->typ_rubber), "");
    }

    if (tectonic_detected()) {
        if (tectonic_active())
            gtk_toggle_button_set_active (prefs->typ_tectonic, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->typ_tectonic), TRUE);
        gtk_widget_set_tooltip_text (GTK_WIDGET(prefs->typ_tectonic), "");
    }

    if (latexmk_detected()) {
        if (latexmk_active())
            gtk_toggle_button_set_active (prefs->typ_latexmk, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->typ_latexmk), TRUE);
        gtk_widget_set_tooltip_text (GTK_WIDGET(prefs->typ_latexmk), "");
    }

    if (latex_method_active ("texpdf")) {
        gtk_toggle_button_set_active (prefs->method_texpdf, TRUE);
    }
    else if (latex_method_active ("texdvipdf")) {
        gtk_toggle_button_set_active (prefs->method_texdvipdf, TRUE);
    }
    else if (latex_method_active ("texdvipspdf")) {
        gtk_toggle_button_set_active (prefs->method_texdvipspdf, TRUE);
    }

    if (!latex_use_shellescaping())
        gtk_toggle_button_set_active (prefs->opt_shellescape, FALSE);
    else {
        gtk_toggle_button_set_active (prefs->opt_shellescape, TRUE);
    }

    if (latex_can_synctex()) {
        if (config_get_boolean ("Compile", "synctex")) {
            gtk_toggle_button_set_active (prefs->opt_synctex, TRUE);
        }
        else {
            gtk_toggle_button_set_active (prefs->opt_synctex, FALSE);
        }
    }
}

static void set_tab_preview_settings (GuPrefsGui* prefs) {
    gboolean pause_status = config_get_boolean ("Compile", "pause");

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->compile_status),
                                  !pause_status);

    if (pause_status) {
        gtk_widget_set_sensitive (GTK_WIDGET (prefs->compile_timer), FALSE);
    }

    gtk_spin_button_set_value (prefs->compile_timer,
                               config_get_integer ("Compile", "timer"));
    // compile scheme:
    if (config_value_as_str_equals ("Compile", "scheme", "real_time"))
        gtk_combo_box_set_active (prefs->compile_scheme, 1);

    // default zoom mode:
    GtkTreeModel* model = 0;
    GtkTreeIter iter;
    gboolean valid = FALSE;
    gint count = 0;

    const gchar *conf_str = config_get_string("Preview", "zoom_mode");

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (prefs->combo_zoom_modes));
    valid = gtk_tree_model_get_iter_first (model, &iter);

    while (valid) {
        const gchar* list_str;
        gtk_tree_model_get (model, &iter, 0, &list_str, -1);
        if (STR_EQU (list_str, conf_str)) {
            gtk_combo_box_set_active (GTK_COMBO_BOX (prefs->combo_zoom_modes), count);
            break;
        }
        ++count;
        valid = gtk_tree_model_iter_next (model, &iter);
    }

    // animated scroll:
    if (config_value_as_str_equals ("Preview", "animated_scroll", "always")) {
        gtk_combo_box_set_active (prefs->combo_animated_scroll, 0);
    } else if (config_value_as_str_equals ("Preview", "animated_scroll", "never")) {
        gtk_combo_box_set_active (prefs->combo_animated_scroll, 2);
    } else {
        gtk_combo_box_set_active (prefs->combo_animated_scroll, 1);
    }

    gtk_spin_button_set_value (prefs->spin_cache_size,
                               config_get_integer ("Preview", "cache_size"));
}

static void set_tab_miscellaneous_settings (GuPrefsGui* prefs) {
    GtkTreeModel* combo_lang = 0;
    GtkTreeIter iter;
    const gchar* lang = 0;
    gint count = 0;
    gboolean valid = FALSE;

    combo_lang = gtk_combo_box_get_model (GTK_COMBO_BOX (prefs->combo_languages));

    lang = config_get_string ("Editor", "spelling_lang");

    valid = gtk_tree_model_get_iter_first (combo_lang, &iter);
    while (valid) {
        const gchar* str_value;
        gtk_tree_model_get (combo_lang, &iter, 0, &str_value, -1);
        if (STR_EQU (lang, str_value)) {
            gtk_combo_box_set_active (GTK_COMBO_BOX (prefs->combo_languages), count);
            break;
        }
        ++count;
        valid = gtk_tree_model_iter_next (combo_lang, &iter);
    }

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->autoexport),
                                  config_get_boolean ("File", "autoexport"));
}

void prefsgui_apply_style_scheme(GuPrefsGui* prefs) {
    const gchar* scheme = config_get_string ("Editor", "style_scheme");
    GList* schemes = editor_list_style_scheme_sorted ();
    GList* schemes_iter = schemes;
    GList* tab = gummi->tabmanager->tabs;
    gint column = 0;
    GtkTreePath* treepath;

    while (schemes_iter) {
        if (STR_EQU (gtk_source_style_scheme_get_id (schemes_iter->data),
                     scheme)) {
            gchar* path = g_strdup_printf ("%d", column);
            treepath = gtk_tree_path_new_from_string (path);
            gtk_tree_view_set_cursor (prefs->styleschemes_treeview, treepath,
                    NULL, FALSE);
            gtk_tree_path_free (treepath);
            g_free (path);
            break;
        }
        ++column;
        schemes_iter = g_list_next (schemes_iter);
    }
    g_list_free (schemes);

    if (schemes && !schemes_iter) {
        treepath = gtk_tree_path_new_from_string ("0");
        gtk_tree_view_set_cursor (prefs->styleschemes_treeview, treepath, NULL,
                FALSE);
        gtk_tree_path_free (treepath);
        while (tab) {
            editor_set_style_scheme_by_id (GU_TAB_CONTEXT (tab->data)->editor,
                                           "classic");
            tab = g_list_next (tab);
        }
    }
}

G_MODULE_EXPORT
void toggle_linenumbers (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    GList* tab = gummi->tabmanager->tabs;

    config_set_boolean ("Editor", "line_numbers", newval);

    while (tab) {
        gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW
                (GU_TAB_CONTEXT (tab->data)->editor->view), newval);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void toggle_highlighting (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    GList* tab = gummi->tabmanager->tabs;

    config_set_boolean ("Editor", "highlighting", newval);

    while (tab) {
        gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW
                (GU_TAB_CONTEXT (tab->data)->editor->view), newval);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void toggle_textwrapping (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    GList* tab = gummi->tabmanager->tabs;

    config_set_boolean ("Editor", "textwrapping", newval);

    if (newval) {
        while (tab) {
            gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW
                    (GU_TAB_CONTEXT (tab->data)->editor->view), GTK_WRAP_CHAR);
            tab = g_list_next (tab);
        }
        gtk_widget_set_sensitive (GTK_WIDGET (gui->prefsgui->wordwrap_button),
                                  TRUE);
    } else {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                (gui->prefsgui->wordwrap_button), FALSE);
        /* NOTE: gtk_text_vew_set_wrap_mode () must be placed after
         * gtk_toggle_button_set_active () since gtk_toggle_button_set_active ()
         * will trigger the 'activate' event of the corresponding button and
         * cause wrapmode to be changed after we set it */
        while (tab) {
            gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW
                    (GU_TAB_CONTEXT (tab->data)->editor->view), GTK_WRAP_NONE);
            tab = g_list_next (tab);
        }
        gtk_widget_set_sensitive
            (GTK_WIDGET (gui->prefsgui->wordwrap_button), FALSE);
    }
}

G_MODULE_EXPORT
void toggle_wordwrapping (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    GList* tab = gummi->tabmanager->tabs;

    config_set_boolean ("Editor", "wordwrapping", newval);

    while (tab) {
        if (newval)
            gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW
                    (GU_TAB_CONTEXT (tab->data)->editor->view), GTK_WRAP_WORD);
        else
            gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW
                    (GU_TAB_CONTEXT (tab->data)->editor->view), GTK_WRAP_CHAR);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void toggle_compilestatus (GtkWidget* widget, void* user) {
    gboolean val = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    config_set_boolean ("Compile", "pause", !val);

    gtk_widget_set_sensitive (GTK_WIDGET(gui->prefsgui->compile_timer), val);
    gtk_toggle_tool_button_set_active (gui->previewgui->preview_pause, !val);
}

G_MODULE_EXPORT
void toggle_spaces_instof_tabs (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_boolean ("Editor", "spaces_instof_tabs", newval);

    GList* tab = gummi->tabmanager->tabs;
    while (tab) {
        gtk_source_view_set_insert_spaces_instead_of_tabs
            (GU_TAB_CONTEXT (tab->data)->editor->view, newval);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void toggle_autoindentation (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_boolean ("Editor", "autoindentation", newval);

    GList* tab = gummi->tabmanager->tabs;
    while (tab) {
        gtk_source_view_set_auto_indent
            (GU_TAB_CONTEXT (tab->data)->editor->view, newval);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void toggle_autosaving (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_boolean ("File", "autosaving", newval);

    if (newval) {
        gtk_widget_set_sensitive (
                GTK_WIDGET (gui->prefsgui->autosave_timer), TRUE);
        gtk_spin_button_set_value (gui->prefsgui->autosave_timer,
                                   config_get_integer ("File", "autosave_timer"));
        iofunctions_reset_autosave (g_active_editor->filename);
    } else {
        gtk_widget_set_sensitive (
                GTK_WIDGET (gui->prefsgui->autosave_timer), FALSE);
        iofunctions_stop_autosave ();
    }
}

G_MODULE_EXPORT
void toggle_autoexport (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_boolean ("File", "autoexport", newval);
}

G_MODULE_EXPORT
void on_prefs_close_clicked (GtkWidget* widget, void* user) {
    GtkTextIter start, end;
    gchar* text = NULL;

    if (gtk_text_buffer_get_modified (gui->prefsgui->default_buffer)) {
        gtk_text_buffer_get_start_iter (gui->prefsgui->default_buffer, &start);
        gtk_text_buffer_get_end_iter (gui->prefsgui->default_buffer, &end);
        text = gtk_text_buffer_get_text (gui->prefsgui->default_buffer, &start,
                                         &end, FALSE);

        utils_set_file_contents (C_WELCOMETEXT, text, -1);
    }
    g_free (text);

    gtk_widget_hide (GTK_WIDGET (gui->prefsgui->prefwindow));
    config_save ();
}

G_MODULE_EXPORT
void on_prefs_reset_clicked (GtkWidget* widget, void* user) {
    config_load_defaults ();
    utils_copy_file (C_DEFAULTTEXT, C_WELCOMETEXT, NULL);

    set_all_tab_settings (gui->prefsgui);
}

G_MODULE_EXPORT
void on_tabwidth_value_changed (GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    config_set_integer ("Editor", "tabwidth", newval);
    
    GList* tab = gummi->tabmanager->tabs;
    while (tab) {
        gtk_source_view_set_tab_width (GU_TAB_CONTEXT (tab->data)->editor->view,
                                      newval);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void on_configure_snippets_clicked (GtkWidget* widget, void* user) {
    snippetsgui_main (gui->snippetsgui);
}

G_MODULE_EXPORT
void on_autosave_value_changed (GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    config_set_integer("File", "autosave_timer", newval);

    iofunctions_reset_autosave (g_active_editor->filename);
}

G_MODULE_EXPORT
void on_compile_value_changed (GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    config_set_integer("Compile", "timer", newval);

    previewgui_reset (gui->previewgui);
}

G_MODULE_EXPORT
void on_cache_size_value_changed (GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    config_set_integer ("Preview", "cache_size", newval);

    g_idle_add((GSourceFunc) run_garbage_collector, gui->previewgui);
}

G_MODULE_EXPORT
void on_editor_font_change (GtkWidget* widget, void* user) {
    PangoFontDescription* font_desc;
    gchar* font_css;
    gchar* font_str;

    font_str = gtk_font_chooser_get_font ( GTK_FONT_CHOOSER (widget));
    config_set_string ("Editor", "font_str", font_str);
    slog (L_INFO, "setting font to %s\n", font_str);
    g_free (font_str);

    font_desc = gtk_font_chooser_get_font_desc (GTK_FONT_CHOOSER (widget));
    font_css  = utils_pango_font_desc_to_css (font_desc);
    config_set_string ("Editor", "font_css", font_css);

    // set new font on all tabs
    GList* tab = gummi->tabmanager->tabs;
    while (tab) {
        editor_set_font (GU_TAB_CONTEXT (tab->data)->editor, font_css);
        tab = g_list_next (tab);
    }
}


G_MODULE_EXPORT
void on_typ_pdflatex_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "typesetter", "pdflatex");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_typ_xelatex_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "typesetter", "xelatex");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_typ_lualatex_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "typesetter", "lualatex");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_typ_rubber_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "typesetter", "rubber");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_typ_tectonic_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "typesetter", "tectonic");
        typesetter_setup ();
    }
}


G_MODULE_EXPORT
void on_typ_latexmk_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "typesetter", "latexmk");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_method_texpdf_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "steps", "texpdf");
        slog (L_INFO, "Changed compile method to \"tex->pdf\"\n");
    }

}

G_MODULE_EXPORT
void on_method_texdvipdf_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "steps", "texdvipdf");
        slog (L_INFO, "Changed compile method to \"tex->dvi->pdf\"\n");
    }
}

G_MODULE_EXPORT
void on_method_texdvipspdf_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_string ("Compile", "steps", "texdvipspdf");
        slog (L_INFO, "Changed compile method to \"tex->dvi->ps->pdf\"\n");
    }
}

G_MODULE_EXPORT
void toggle_shellescape (GtkToggleButton* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_boolean ("Compile", "shellescape", newval);
}

G_MODULE_EXPORT
void on_synctex_toggled (GtkToggleButton* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_boolean ("Compile", "synctex", newval);
    gtk_widget_set_sensitive (GTK_WIDGET (gui->menu_autosync), newval);
}

G_MODULE_EXPORT
void on_combo_language_changed (GtkComboBoxText* widget, void* user) {

    gchar* selected = gtk_combo_box_text_get_active_text (widget);
    GList* tab = gummi->tabmanager->tabs;
    config_set_string ("Editor", "spelling_lang", selected);

    if (config_get_boolean ("Editor", "spelling")) {
        while (tab) {
            editor_activate_spellchecking (GU_TAB_CONTEXT (tab->data)->editor,
                                           FALSE);
            editor_activate_spellchecking (GU_TAB_CONTEXT (tab->data)->editor,
                                           TRUE);
            tab = g_list_next (tab);
        }
    }
    g_free(selected);
}

G_MODULE_EXPORT
void on_combo_compilescheme_changed (GtkWidget* widget, void* user) {
    gint selected = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    gchar scheme[][16] = { "on_idle", "real_time" };
    slog (L_INFO, "compile scheme set to %s\n", scheme[selected]);
    config_set_string ("Compile", "scheme", scheme[selected]);
    previewgui_reset (gui->previewgui);
}

G_MODULE_EXPORT
void on_combo_zoom_modes_changed (GtkWidget* widget, void* user) {
    gint selected = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    // lazily duplicating this again for now (TODO fix in 0.8.1)
    gchar scheme[][16] = {"Best Fit", "Fit Page Width", "50%", "70%", "85%",
                          "100%", "125%", "150%", "200%", "300%", "400%"};
    config_set_string ("Preview", "zoom_mode", scheme[selected]);
}

G_MODULE_EXPORT
void on_combo_animated_scroll_changed (GtkWidget* widget, void* user) {
    gint selected = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    gchar scheme[][16] = { "always", "autosync", "never" };
    config_set_string ("Preview", "animated_scroll", scheme[selected]);
}

G_MODULE_EXPORT
void on_styleschemes_treeview_cursor_changed (GtkTreeView* treeview, void* user) {
    gchar* id = NULL;
    gchar* name = NULL;
    GList* tab;
    GtkTreeIter iter;

    GtkTreeModel* model = GTK_TREE_MODEL (gtk_tree_view_get_model (treeview));
    GtkTreeSelection* selection = gtk_tree_view_get_selection (treeview);

    gtk_tree_selection_get_selected (selection, &model, &iter);
    gtk_tree_model_get (model, &iter, 0, &name, 1, &id, -1);
    tab = gummi->tabmanager->tabs;
    while (tab) {
        editor_set_style_scheme_by_id (GU_TAB_CONTEXT (tab->data)->editor, id);
        tab = g_list_next (tab);
    }
    config_set_string ("Editor", "style_scheme", id);
}
