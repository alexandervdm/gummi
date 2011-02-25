/**
 * @file   gui-prefs.c
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

#include "gui-prefs.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "configfile.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "utils.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuPrefsGui* prefsgui_init(GtkWidget* mainwindow) {
    GuPrefsGui* p = g_new0(GuPrefsGui, 1);
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, DATADIR"/prefs.glade", NULL);
    gtk_builder_set_translation_domain(builder, PACKAGE);

    p->prefwindow =
        GTK_WIDGET(gtk_builder_get_object(builder, "prefwindow"));
    p->notebook =
        GTK_NOTEBOOK(gtk_builder_get_object(builder, "notebook1"));
    p->textwrap_button =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "textwrapping"));
    p->wordwrap_button =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "wordwrapping"));
    p->line_numbers =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "line_numbers")); 
    p->highlighting =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "highlighting")); 
    p->tabwidth =
        GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "tabwidth"));
    p->spaces_instof_tabs =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "spaces_instof_tabs"));
    p->autoindentation =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autoindentation"));
    p->autosaving =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autosaving")); 
    p->compile_status =
        GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "compile_status")); 
    p->autosave_timer =
        GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "autosave_timer"));
    p->combo_languages =
        GTK_COMBO_BOX(gtk_builder_get_object(builder, "combo_languages"));
    p->list_languages =
        GTK_LIST_STORE(gtk_builder_get_object(builder, "list_languages"));
    p->styleschemes_treeview =
        GTK_TREE_VIEW(gtk_builder_get_object(builder, "styleschemes_treeview"));
    p->list_styleschemes =
        GTK_LIST_STORE(gtk_builder_get_object(builder, "list_styleschemes"));
    p->default_text =
        GTK_TEXT_VIEW(gtk_builder_get_object(builder, "default_text"));
    p->default_buffer = 
        gtk_text_view_get_buffer(p->default_text);
    p->typesetter =
        GTK_COMBO_BOX(gtk_builder_get_object(builder, "combo_typesetter"));
    p->custom_typesetter =
        GTK_ENTRY(gtk_builder_get_object(builder, "custom_typesetter"));
    p->extra_flags = 
        GTK_ENTRY(gtk_builder_get_object(builder, "extra_flags"));
    p->editor_font =
        GTK_FONT_BUTTON(gtk_builder_get_object(builder, "editor_font"));
    p->compile_scheme =
        GTK_COMBO_BOX(gtk_builder_get_object(builder, "combo_compilescheme"));
    p->compile_timer =
        GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "compile_timer"));

    p->view_box = GTK_VBOX(gtk_builder_get_object(builder, "view_box"));
    p->editor_box = GTK_HBOX(gtk_builder_get_object(builder, "editor_box"));
    p->compile_box = GTK_HBOX(gtk_builder_get_object(builder, "compile_box"));
    p->commandbox = GTK_HBOX(gtk_builder_get_object(builder, "commandbox"));

    gtk_window_set_transient_for(GTK_WINDOW(p->prefwindow), 
            GTK_WINDOW(mainwindow));

    /* list available style schemes */
    GList* schemes = editor_list_style_scheme_sorted(gummi->editor);
    GList* schemes_iter = schemes;
    gchar* desc = NULL;
    GtkTreeIter iter;
    while (schemes_iter) {
        desc = g_markup_printf_escaped("<b>%s</b> - %s",
                gtk_source_style_scheme_get_name(schemes_iter->data),
                gtk_source_style_scheme_get_description(schemes_iter->data));
        gtk_list_store_append(p->list_styleschemes, &iter);
        gtk_list_store_set(p->list_styleschemes, &iter,
                0, desc,
                1, gtk_source_style_scheme_get_id(schemes_iter->data), -1);
        schemes_iter = g_list_next(schemes_iter);
        g_free(desc);
    }
    g_list_free(schemes);

#ifdef USE_GTKSPELL
    /* list available languages */
    gchar* ptr = 0;

    Tuple2 pret = utils_popen_r("enchant-lsmod -list-dicts");

    ptr = strtok((gchar*)pret.second, " \n");
    while (ptr) {
        GtkTreeIter iter;
        if (ptr[0] != '(') {
            gtk_list_store_append(p->list_languages, &iter);
            gtk_list_store_set(p->list_languages, &iter, 0, ptr, -1);
        }
        ptr = strtok(NULL, " \n");
    }
    gtk_combo_box_set_active(p->combo_languages, 0);
    g_free((gchar*)pret.second);
#else
    /* remove gtkspell related GUIs if not used */
    GtkHBox* hbox11 = GTK_HBOX(gtk_builder_get_object(builder, "hbox11"));
    GtkHBox* hbox10 = GTK_HBOX(gtk_builder_get_object(builder, "hbox10"));
    GtkLabel* label9 = GTK_LABEL(gtk_builder_get_object(builder, "label9"));
    gtk_container_remove(GTK_CONTAINER(hbox11), GTK_WIDGET(label9));
    gtk_container_remove(GTK_CONTAINER(hbox10), GTK_WIDGET(p->combo_languages));
#endif

    gtk_builder_connect_signals(builder, NULL);

    return p;
}

void prefsgui_main(GuPrefsGui* prefs) {
    prefsgui_set_current_settings(prefs);
    gtk_widget_show_all(GTK_WIDGET(prefs->prefwindow));
}

void prefsgui_set_current_settings(GuPrefsGui* prefs) {
    /* set font */
    GtkTreeModel* combo_lang = 0;
    GtkTreeIter iter;
    const gchar* lang = 0;
    gint count = 0;
    gboolean value = FALSE, valid = FALSE;
    const gchar* font = config_get_value("font");

    PangoFontDescription* font_desc = pango_font_description_from_string(font);
    gtk_widget_modify_font(GTK_WIDGET(prefs->default_text), font_desc);
    gtk_widget_modify_font(GTK_WIDGET(gummi->editor->sourceview), font_desc);
    pango_font_description_free(font_desc);

    /* set all checkboxs */
    value = (gboolean)config_get_value("textwrapping");
    if (value) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->textwrap_button),
                value);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->wordwrap_button),
                (gboolean)config_get_value("wordwrapping"));
    } else
        gtk_widget_set_sensitive(GTK_WIDGET(prefs->wordwrap_button), FALSE);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->line_numbers),
            (gboolean)config_get_value("line_numbers"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->highlighting),
            (gboolean)config_get_value("highlighting"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->autosaving),
            (gboolean)config_get_value("autosaving"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->compile_status),
            (gboolean)config_get_value("compile_status"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->spaces_instof_tabs),
            (gboolean)config_get_value("spaces_instof_tabs"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prefs->autoindentation),
            (gboolean)config_get_value("autoindentation"));

    if (!config_get_value("autosaving"))
        gtk_widget_set_sensitive(GTK_WIDGET(prefs->autosave_timer), FALSE);

    if (!config_get_value("compile_status"))
        gtk_widget_set_sensitive(GTK_WIDGET(prefs->compile_timer), FALSE);

    /* set spin button */
    gtk_spin_button_set_value(prefs->autosave_timer,
            atoi(config_get_value("autosave_timer")));
    gtk_spin_button_set_value(prefs->compile_timer,
            atoi(config_get_value("compile_timer")));
    gtk_spin_button_set_value(prefs->tabwidth,
            atoi(config_get_value("tabwidth")));

    gtk_font_button_set_font_name(prefs->editor_font,
            config_get_value("font"));
    gtk_text_buffer_set_text(prefs->default_buffer,
            config_get_value("welcome"), strlen(config_get_value("welcome")));

    /* set combo boxes */
    /* typesetter */
    const gchar* typesetter = config_get_value("typesetter");
    gtk_widget_hide(GTK_WIDGET(gui->prefsgui->commandbox));
    if (0 == strcmp(typesetter, "pdflatex"))
        gtk_combo_box_set_active(prefs->typesetter, 0);
    else if (0 == strcmp(typesetter, "xelatex"))
        gtk_combo_box_set_active(prefs->typesetter, 1);
    else {
        gtk_widget_show(GTK_WIDGET(gui->prefsgui->commandbox));
        gtk_entry_set_text(prefs->custom_typesetter, typesetter);
        gtk_combo_box_set_active(prefs->typesetter, 2);
    }

    /* compile scheme */
    if (0 == strcmp(config_get_value("compile_scheme"), "real_time"))
        gtk_combo_box_set_active(prefs->compile_scheme, 1);

    combo_lang = gtk_combo_box_get_model(prefs->combo_languages);

    lang = config_get_value("spell_language");
    valid = gtk_tree_model_get_iter_first(combo_lang, &iter);
    while (valid) {
        const gchar* str_value;
        gtk_tree_model_get(combo_lang, &iter, 0, &str_value, -1);
        if (0 == strcmp(lang, str_value)) {
            gtk_combo_box_set_active(prefs->combo_languages, count);
            break;
        }
        ++count;
        valid = gtk_tree_model_iter_next(combo_lang, &iter);
    }

    /* set style scheme */
    const gchar* scheme = config_get_value("style_scheme");
    GList* schemes = editor_list_style_scheme_sorted(gummi->editor);
    GList* schemes_iter = schemes;
    gint column = 0;
    GtkTreePath* treepath;
    while (schemes_iter) {
        if (0 == strcmp(gtk_source_style_scheme_get_id(schemes_iter->data),
                    scheme)) {
            gchar* path = g_strdup_printf("%d", column);
            treepath = gtk_tree_path_new_from_string(path);
            gtk_tree_view_set_cursor(prefs->styleschemes_treeview, treepath,
                    NULL, FALSE);
            gtk_tree_path_free(treepath);
            g_free(path);
            break;
        }
        ++column;
        schemes_iter = g_list_next(schemes_iter);
    }
    g_list_free(schemes);

    if (schemes && !schemes_iter) {
        treepath = gtk_tree_path_new_from_string("0");
        gtk_tree_view_set_cursor(prefs->styleschemes_treeview, treepath, NULL,
                FALSE);
        gtk_tree_path_free(treepath);
        editor_set_style_scheme_by_id(gummi->editor, "classic");
    }

    /* set extra flags */
    gtk_entry_set_text(prefs->extra_flags, config_get_value("extra_flags"));
}

void toggle_linenumbers(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("line_numbers", newval? "True": "False");
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(
                gummi->editor->sourceview), newval);
}

void toggle_highlighting(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("highlighting", newval? "True": "False");
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(
                gummi->editor->sourceview), newval);
}

void toggle_textwrapping(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("textwrapping", newval? "True": "False");
    if (newval) {
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_CHAR);
        gtk_widget_set_sensitive(
                GTK_WIDGET(gui->prefsgui->wordwrap_button), TRUE);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                    gui->prefsgui->wordwrap_button), FALSE);
        /* NOTE: gtk_text_vew_set_wrap_mode() must be placed after
         * gtk_toggle_button_set_active() since gtk_toggle_button_set_active()
         * will trigger the 'activate' event of the corresponding button and
         * cause wrapmode to be changed after we set it */
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_NONE);
        gtk_widget_set_sensitive(
                GTK_WIDGET(gui->prefsgui->wordwrap_button), FALSE);
    }
}

void toggle_wordwrapping(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("wordwrapping", newval? "True": "False");
    if (newval)
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_WORD);
    else
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_CHAR);
}

void toggle_compilestatus(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("compile_status", newval? "True": "False");
    gtk_widget_set_sensitive(GTK_WIDGET(gui->prefsgui->compile_timer), newval);
    gtk_toggle_tool_button_set_active(gui->previewoff, !newval);
}

void toggle_spaces_instof_tabs(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("spaces_instof_tabs", newval? "True": "False");
    gtk_source_view_set_insert_spaces_instead_of_tabs(
            gummi->editor->sourceview, newval);
}

void toggle_autoindentation(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("autoindentation", newval? "True": "False");
    gtk_source_view_set_auto_indent(gummi->editor->sourceview, newval);
}

void toggle_autosaving(GtkWidget* widget, void* user) {
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("autosaving", newval? "True": "False");
    if (newval) {
        gtk_widget_set_sensitive(
                GTK_WIDGET(gui->prefsgui->autosave_timer), TRUE);
        gint time = atoi(config_get_value("autosave_timer"));
        gtk_spin_button_set_value(gui->prefsgui->autosave_timer, time);
        iofunctions_reset_autosave(gummi->editor->filename);
    } else {
        gtk_widget_set_sensitive(
                GTK_WIDGET(gui->prefsgui->autosave_timer), FALSE);
        iofunctions_stop_autosave();
    }
}

void on_prefs_close_clicked(GtkWidget* widget, void* user) {
    GtkTextIter start, end;
    gchar* text = NULL;
    gtk_text_buffer_get_start_iter(gui->prefsgui->default_buffer, &start);
    gtk_text_buffer_get_end_iter(gui->prefsgui->default_buffer, &end);
    text = gtk_text_buffer_get_text(gui->prefsgui->default_buffer, &start,
            &end, FALSE);
    config_set_value("welcome", text);
    g_free(text);

    /* set custom typesetter */
    const gchar* typesetter = gtk_entry_get_text(
            gui->prefsgui->custom_typesetter);
    if (strlen(typesetter))
        config_set_value("typesetter", typesetter);

    const gchar* flags = gtk_entry_get_text(gui->prefsgui->extra_flags);
    config_set_value("extra_flags", flags);

    gtk_widget_hide(GTK_WIDGET(gui->prefsgui->prefwindow));
}

void on_prefs_reset_clicked(GtkWidget* widget, void* user) {
    config_set_default();
    prefsgui_set_current_settings(gui->prefsgui);
}

void on_tabwidth_value_changed(GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar buf[16];

    config_set_value("tabwidth", g_ascii_dtostr(buf, 16, (double)newval));
    gtk_source_view_set_tab_width(gummi->editor->sourceview, newval);
}

void on_configure_snippets_clicked(GtkWidget* widget, void* user) {
    snippetsgui_main(gui->snippetsgui);
}

void on_autosave_value_changed(GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar buf[16];

    config_set_value("autosave_timer", g_ascii_dtostr(buf, 16, (double)newval));
    iofunctions_reset_autosave(gummi->editor->filename);
}

void on_compile_value_changed(GtkWidget* widget, void* user) {
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar buf[16];

    config_set_value("compile_timer", g_ascii_dtostr(buf, 16, (double)newval));
    previewgui_reset(gui->previewgui);
}

void on_editor_font_set(GtkWidget* widget, void* user) {
    const gchar* font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(widget));
    slog(L_INFO, "setting font to %s\n", font);
    config_set_value("font", font);
    PangoFontDescription* font_desc = pango_font_description_from_string(font);
    gtk_widget_modify_font(GTK_WIDGET(gummi->editor->sourceview), font_desc);
    pango_font_description_free(font_desc);
}

void on_combo_typesetter_changed(GtkWidget* widget, void* user) {
    gint selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    const gchar typesetter[][16] = { "pdflatex", "xelatex", "Customize" };
    if (selected != 2) {
        config_set_value("typesetter", typesetter[selected]);
        gtk_entry_set_text(gui->prefsgui->custom_typesetter, "");
        gtk_widget_hide(GTK_WIDGET(gui->prefsgui->commandbox));
    } else
        gtk_widget_show(GTK_WIDGET(gui->prefsgui->commandbox));
}

void on_combo_language_changed(GtkWidget* widget, void* user) {
#ifdef USE_GTKSPELL
    gchar* selected = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
    config_set_value("spell_language", selected);
    if (config_get_value("spelling")) {
        editor_activate_spellchecking(gummi->editor, FALSE);
        editor_activate_spellchecking(gummi->editor, TRUE);
    }
#endif
}

void on_combo_compilescheme_changed(GtkWidget* widget, void* user) {
    gint selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    const gchar scheme[][16] = { "on_idle", "real_time" };
    slog(L_INFO, "compile scheme set to %s\n", scheme[selected]);
    config_set_value("compile_scheme", scheme[selected]);
    previewgui_reset(gui->previewgui);
}

void on_styleschemes_treeview_cursor_changed(GtkTreeView* treeview, void* user)
{
    GtkTreeIter iter;
    gchar* name;
    gchar* id;
    GtkTreeModel* model = GTK_TREE_MODEL(gtk_tree_view_get_model(treeview));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(treeview);

    gtk_tree_selection_get_selected(selection, &model, &iter);
    gtk_tree_model_get(model, &iter, 0, &name, 1, &id, -1);
    editor_set_style_scheme_by_id(gummi->editor, id);
    config_set_value("style_scheme", id);
}
