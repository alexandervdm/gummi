/**
 * @file   gui-prefs.c
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
    gchar* ui = g_build_filename (DATADIR, "ui", "prefs.glade", NULL);
    gtk_builder_add_from_file (builder, ui, NULL);
    gtk_builder_set_translation_domain (builder, PACKAGE);
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
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "combo_languages"));
    p->list_languages =
        GTK_LIST_STORE (gtk_builder_get_object (builder, "list_languages"));
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
    p->typ_rubber =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "typ_rubber"));
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
        
    p->combo_animated_scroll =
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "combo_animated_scroll"));
    p->spin_cache_size =
        GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "spin_cache_size"));
        
    p->view_box = GTK_VBOX (gtk_builder_get_object (builder, "view_box"));
    p->editor_box = GTK_HBOX (gtk_builder_get_object (builder, "editor_box"));
    p->compile_box = GTK_HBOX (gtk_builder_get_object (builder, "compile_box"));

    gtk_window_set_transient_for (GTK_WINDOW (p->prefwindow), mainwindow);

#ifdef USE_GTKSPELL
    /* list available languages */

    if (g_file_test ("enchant-lsmod", G_FILE_TEST_EXISTS)) {
        Tuple2 pret = utils_popen_r ("enchant-lsmod -list-dicts", NULL);
    
        if (pret.second != NULL) {
            gchar** output = g_strsplit((gchar*)pret.second, "\n", BUFSIZ);
            gchar** elems = NULL;
            int i;
    
            for(i = 0; output[i] != NULL; i++) {
                GtkTreeIter iter;
                elems = g_strsplit (output[i], " ", BUFSIZ);
                if (elems[0] != NULL) {
                    gtk_list_store_append (p->list_languages, &iter);
                    gtk_list_store_set (p->list_languages, &iter, 0, elems[0], -1);
                }
            }
        g_strfreev(output);
        g_strfreev(elems);
        }
        gtk_combo_box_set_active (p->combo_languages, 0);
        g_free ((gchar*)pret.second);
    }
#else
    /* desensitise gtkspell related GUI elements if not used */
    GtkWidget* box = GTK_WIDGET (
		gtk_builder_get_object (builder, "box_spellcheck"));
    gtk_widget_set_sensitive (box, FALSE);
#endif
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
    gboolean value = FALSE;

    value = TO_BOOL (config_get_value ("textwrapping"));
    if (value) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(prefs->textwrap_button),
                value);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(prefs->wordwrap_button),
                TO_BOOL (config_get_value ("wordwrapping")));
    } else
        gtk_widget_set_sensitive (GTK_WIDGET (prefs->wordwrap_button), FALSE);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->line_numbers),
            TO_BOOL (config_get_value ("line_numbers")));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->highlighting),
            TO_BOOL (config_get_value ("highlighting")));    
    
}

static void set_tab_editor_settings (GuPrefsGui* prefs) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->autoindentation),
            TO_BOOL (config_get_value ("autoindentation")));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->spaces_instof_tabs),
            TO_BOOL (config_get_value ("spaces_instof_tabs")));
    gtk_spin_button_set_value (prefs->tabwidth,
                               atoi (config_get_value ("tabwidth")));
    gtk_spin_button_set_value (prefs->autosave_timer,
                               atoi (config_get_value ("autosave_timer")));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->autosaving),
            TO_BOOL (config_get_value ("autosaving")));
    if (!config_get_value ("autosaving"))
        gtk_widget_set_sensitive (GTK_WIDGET (prefs->autosave_timer), FALSE);
}

static void set_tab_fontcolor_settings (GuPrefsGui* prefs) {
    const gchar* font = config_get_value ("font");

    PangoFontDescription* font_desc = pango_font_description_from_string (font);
    gtk_widget_modify_font (GTK_WIDGET (prefs->default_text), font_desc);
    pango_font_description_free (font_desc);
    
    gtk_font_button_set_font_name (prefs->editor_font, 
                                        config_get_value ("font"));
    prefsgui_apply_style_scheme(prefs);
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
    
    if (rubber_detected()) {
        if (rubber_active()) 
            gtk_toggle_button_set_active (prefs->typ_rubber, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET(prefs->typ_rubber), TRUE);
        gtk_widget_set_tooltip_text (GTK_WIDGET(prefs->typ_rubber), "");
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
        if (config_get_value ("synctex")) {
            gtk_toggle_button_set_active (prefs->opt_synctex, TRUE);
        }
        else {
            gtk_toggle_button_set_active (prefs->opt_synctex, FALSE);
        }
    }
}

static void set_tab_preview_settings (GuPrefsGui* prefs) {

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->compile_status),
            TO_BOOL (config_get_value ("compile_status")));

    if (!config_get_value ("compile_status"))
        gtk_widget_set_sensitive (GTK_WIDGET (prefs->compile_timer), FALSE);
        
    gtk_spin_button_set_value (prefs->compile_timer,
                               atoi (config_get_value ("compile_timer")));
    /* compile scheme */
    if (STR_EQU (config_get_value ("compile_scheme"), "real_time"))
        gtk_combo_box_set_active (prefs->compile_scheme, 1);
                               
    if (STR_EQU (config_get_value ("animated_scroll"), "always")) {
        gtk_combo_box_set_active (prefs->combo_animated_scroll, 0);
    } else if (STR_EQU (config_get_value ("animated_scroll"), "never")) {
        gtk_combo_box_set_active (prefs->combo_animated_scroll, 2);
    } else {
        gtk_combo_box_set_active (prefs->combo_animated_scroll, 1);
    }
    
    gtk_spin_button_set_value (prefs->spin_cache_size,
                               atoi (config_get_value ("cache_size")));
}

static void set_tab_miscellaneous_settings (GuPrefsGui* prefs) {
    GtkTreeModel* combo_lang = 0;
    GtkTreeIter iter;
    const gchar* lang = 0;
    gint count = 0;
    gboolean valid = FALSE;

    combo_lang = gtk_combo_box_get_model (prefs->combo_languages);

    lang = config_get_value ("spell_language");
    valid = gtk_tree_model_get_iter_first (combo_lang, &iter);
    while (valid) {
        const gchar* str_value;
        gtk_tree_model_get (combo_lang, &iter, 0, &str_value, -1);
        if (STR_EQU (lang, str_value)) {
            gtk_combo_box_set_active (prefs->combo_languages, count);
            break;
        }
        ++count;
        valid = gtk_tree_model_iter_next (combo_lang, &iter);
    }    
    
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->autoexport),
            TO_BOOL (config_get_value ("autoexport")));

}

void prefsgui_apply_style_scheme(GuPrefsGui* prefs) {
    const gchar* scheme = config_get_value ("style_scheme");
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

    config_set_value ("line_numbers", newval? "True": "False");
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

    config_set_value ("highlighting", newval? "True": "False");
    
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

    config_set_value ("textwrapping", newval? "True": "False");
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

    config_set_value ("wordwrapping", newval? "True": "False");
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
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_value ("compile_status", newval? "True": "False");
    gtk_widget_set_sensitive (GTK_WIDGET(gui->prefsgui->compile_timer), newval);
    gtk_toggle_tool_button_set_active (gui->previewoff, !newval);
}

G_MODULE_EXPORT
void toggle_spaces_instof_tabs (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    GList* tab = gummi->tabmanager->tabs;

    config_set_value ("spaces_instof_tabs", newval? "True": "False");
    while (tab) {
        gtk_source_view_set_insert_spaces_instead_of_tabs
            (GU_TAB_CONTEXT (tab->data)->editor->view, newval);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void toggle_autoindentation (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    GList* tab = gummi->tabmanager->tabs;

    config_set_value ("autoindentation", newval? "True": "False");
    while (tab) {
        gtk_source_view_set_auto_indent
            (GU_TAB_CONTEXT (tab->data)->editor->view, newval);
        tab = g_list_next (tab);
    }
}

G_MODULE_EXPORT
void toggle_autosaving (GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    config_set_value ("autosaving", newval? "True": "False");
    if (newval) {
        gtk_widget_set_sensitive (
                GTK_WIDGET (gui->prefsgui->autosave_timer), TRUE);
        gint time = atoi (config_get_value ("autosave_timer"));
        gtk_spin_button_set_value (gui->prefsgui->autosave_timer, time);
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
    config_set_value ("autoexport", newval? "True": "False");
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
}

G_MODULE_EXPORT
void on_prefs_reset_clicked (GtkWidget* widget, void* user) {
    config_set_default ();
    utils_copy_file (C_DEFAULTTEXT, C_WELCOMETEXT, NULL);
    
    set_all_tab_settings (gui->prefsgui);
}

G_MODULE_EXPORT
void on_tabwidth_value_changed (GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    gchar buf[16];
    GList* tab = gummi->tabmanager->tabs;

    config_set_value ("tabwidth", g_ascii_dtostr (buf, 16, (double)newval));
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
    gchar buf[16];

    config_set_value("autosave_timer", g_ascii_dtostr(buf, 16, (double)newval));
    iofunctions_reset_autosave (g_active_editor->filename);
}

G_MODULE_EXPORT
void on_compile_value_changed (GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    gchar buf[16];

    config_set_value("compile_timer", g_ascii_dtostr (buf, 16, (double)newval));
    previewgui_reset (gui->previewgui);
}

G_MODULE_EXPORT
void on_cache_size_value_changed(GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
    gchar buf[16];

    config_set_value("cache_size", g_ascii_dtostr (buf, 16, (double)newval));
    
     
    g_idle_add((GSourceFunc) run_garbage_collector, gui->previewgui);
}

G_MODULE_EXPORT
void on_editor_font_set (GtkWidget* widget, void* user) {
    const gchar* font = gtk_font_button_get_font_name(GTK_FONT_BUTTON (widget));
    PangoFontDescription* font_desc = pango_font_description_from_string (font);
    GList* tab = gummi->tabmanager->tabs;

    slog (L_INFO, "setting font to %s\n", font);
    config_set_value ("font", font);

    while (tab) {
        gtk_widget_modify_font (GTK_WIDGET
                (GU_TAB_CONTEXT (tab->data)->editor->view), font_desc);
        tab = g_list_next (tab);
    }
    pango_font_description_free (font_desc);
}


G_MODULE_EXPORT
void on_typ_pdflatex_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_value ("typesetter", "pdflatex");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_typ_xelatex_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_value ("typesetter", "xelatex");
        typesetter_setup ();
    }

}

G_MODULE_EXPORT
void on_typ_rubber_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_value ("typesetter", "rubber");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_typ_latexmk_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_value ("typesetter", "latexmk");
        typesetter_setup ();
    }
}

G_MODULE_EXPORT
void on_method_texpdf_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_value ("compile_steps", "texpdf");
        slog (L_INFO, "Changed compile method to \"tex->pdf\"\n");
    }
    
}

G_MODULE_EXPORT
void on_method_texdvipdf_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_value ("compile_steps", "texdvipdf");
        slog (L_INFO, "Changed compile method to \"tex->dvi->pdf\"\n");
    }
}

G_MODULE_EXPORT
void on_method_texdvipspdf_toggled (GtkToggleButton* widget, void* user) {
    if (gtk_toggle_button_get_active (widget)) {
        config_set_value ("compile_steps", "texdvipspdf");
        slog (L_INFO, "Changed compile method to \"tex->dvi->ps->pdf\"\n");
    }
}

G_MODULE_EXPORT
void toggle_shellescape (GtkToggleButton* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_value ("shellescape", newval? "True": "False");
}

G_MODULE_EXPORT
void on_synctex_toggled (GtkToggleButton* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    config_set_value ("synctex", newval? "True": "False");
    gtk_widget_set_sensitive (GTK_WIDGET (gui->menu_autosync), newval);
}

G_MODULE_EXPORT
void on_combo_language_changed (GtkWidget* widget, void* user) {
#ifdef USE_GTKSPELL
    gchar* selected = gtk_combo_box_get_active_text (GTK_COMBO_BOX (widget));
    GList* tab = gummi->tabmanager->tabs;
    config_set_value ("spell_language", selected);

    if (config_get_value ("spelling")) {
        while (tab) {
            editor_activate_spellchecking (GU_TAB_CONTEXT (tab->data)->editor,
                                           FALSE);
            editor_activate_spellchecking (GU_TAB_CONTEXT (tab->data)->editor,
                                           TRUE);
            tab = g_list_next (tab);
        }
    }
#endif
}

G_MODULE_EXPORT
void on_combo_compilescheme_changed (GtkWidget* widget, void* user) {
    gint selected = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    const gchar scheme[][16] = { "on_idle", "real_time" };
    slog (L_INFO, "compile scheme set to %s\n", scheme[selected]);
    config_set_value ("compile_scheme", scheme[selected]);
    previewgui_reset (gui->previewgui);
}

G_MODULE_EXPORT
void on_combo_animated_scroll_changed (GtkWidget* widget, void* user) {
    gint selected = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    const gchar scheme[][16] = { "always", "autosync", "never" };
    config_set_value ("animated_scroll", scheme[selected]);
}

G_MODULE_EXPORT
void on_styleschemes_treeview_cursor_changed (GtkTreeView* treeview, void* user) {
    gchar* id = NULL;
    gchar* name = NULL;
    GList* tab = gummi->tabmanager->tabs;
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
    config_set_value ("style_scheme", id);
}
