/**
 * @file   prefs-gui.c
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

#include "prefs-gui.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "configfile.h"
#include "environment.h"
#include "utils.h"

extern Gummi* gummi;

GuPrefsGui* prefsgui_init(GtkWindow* mainwindow) {
    L_F_DEBUG;
    GuPrefsGui* p = (GuPrefsGui*)g_malloc(sizeof(GuPrefsGui));
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
    p->default_text =
        GTK_TEXT_VIEW(gtk_builder_get_object(builder, "default_text"));
    p->default_buffer = 
        gtk_text_view_get_buffer(p->default_text);
    p->typesetter =
        GTK_COMBO_BOX(gtk_builder_get_object(builder, "combo_typesetter"));
    p->editor_font =
        GTK_FONT_BUTTON(gtk_builder_get_object(builder, "editor_font"));
    p->compile_scheme =
        GTK_COMBO_BOX(gtk_builder_get_object(builder, "combo_compilescheme"));
    p->compile_timer =
        GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "compile_timer"));

    p->view_box =
        GTK_VBOX(gtk_builder_get_object(builder, "view_box"));
    p->editor_box =
        GTK_HBOX(gtk_builder_get_object(builder, "editor_box"));
    p->compile_box =
        GTK_HBOX(gtk_builder_get_object(builder, "compile_box"));

    p->changeimg =
        GTK_IMAGE(gtk_builder_get_object(builder, "changeimg"));
    p->changelabel =
        GTK_LABEL(gtk_builder_get_object(builder, "changelabel"));

    gtk_window_set_transient_for(GTK_WINDOW(p->prefwindow), 
            GTK_WINDOW(mainwindow));

#ifdef USE_GTKSPELL
    /* list available languages */
    gchar* ptr = 0;

    pdata pret = utils_popen_r("enchant-lsmod -list-dicts");

    ptr = strtok(pret.data, " \n");
    while (ptr) {
        GtkTreeIter iter;
        if (ptr[0] != '(') {
            gtk_list_store_append(p->list_languages, &iter);
            gtk_list_store_set(p->list_languages, &iter, 0, ptr, -1);
        }
        ptr = strtok(NULL, " \n");
    }
    gtk_combo_box_set_active(p->combo_languages, 0);
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

void prefsgui_main(void) {
    L_F_DEBUG;
    prefsgui_set_current_settings(gummi->gui->prefsgui);
    gtk_widget_show_all(GTK_WIDGET(gummi->gui->prefsgui->prefwindow));
}

void prefsgui_set_current_settings(GuPrefsGui* prefs) {
    L_F_DEBUG;
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
    if (0 == strcmp(config_get_value("typesetter"), "xelatex"))
        gtk_combo_box_set_active(prefs->typesetter, 1);

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
}

void toggle_linenumbers(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("line_numbers", newval? "True": "False");
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(
                gummi->editor->sourceview), newval);
}

void toggle_highlighting(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("highlighting", newval? "True": "False");
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(
                gummi->editor->sourceview), newval);
}

void toggle_textwrapping(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("textwrapping", newval? "True": "False");
    if (newval) {
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_CHAR);
        gtk_widget_set_sensitive(
                GTK_WIDGET(gummi->gui->prefsgui->wordwrap_button), TRUE);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                    gummi->gui->prefsgui->wordwrap_button), FALSE);
        /* NOTE: gtk_text_vew_set_wrap_mode() must be placed after
         * gtk_toggle_button_set_active() since gtk_toggle_button_set_active()
         * will trigger the 'activate' event of the corresponding button and
         * cause wrapmode to be changed after we set it */
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_NONE);
        gtk_widget_set_sensitive(
                GTK_WIDGET(gummi->gui->prefsgui->wordwrap_button), FALSE);
    }
}

void toggle_wordwrapping(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    config_set_value("wordwrapping", newval? "True": "False");
    if (newval)
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_WORD);
    else
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_CHAR);
}

void toggle_compilestatus(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("compile_status", newval? "True": "False");
    if (newval) {
        gtk_widget_set_sensitive(
                GTK_WIDGET(gummi->gui->prefsgui->compile_timer), TRUE);
        gtk_toggle_tool_button_set_active(gummi->gui->previewoff, FALSE);
    } else {
        gtk_widget_set_sensitive(
                GTK_WIDGET(gummi->gui->prefsgui->compile_timer), FALSE);
        gtk_toggle_tool_button_set_active(gummi->gui->previewoff, TRUE);
    }
}

void toggle_spaces_instof_tabs(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("spaces_instof_tabs", newval? "True": "False");
    gtk_source_view_set_insert_spaces_instead_of_tabs(
            gummi->editor->sourceview, newval);
}

void toggle_autoindentation(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("autoindentation", newval? "True": "False");
    gtk_source_view_set_auto_indent(gummi->editor->sourceview, newval);
}

void toggle_autosaving(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gboolean newval = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    config_set_value("autosaving", newval? "True": "False");
    if (newval) {
        gtk_widget_set_sensitive(
                GTK_WIDGET(gummi->gui->prefsgui->autosave_timer), TRUE);
        gint time = atoi(config_get_value("autosave_timer"));
        gtk_spin_button_set_value(gummi->gui->prefsgui->autosave_timer, time);
        iofunctions_reset_autosave(gummi->finfo->filename);
    } else {
        gtk_widget_set_sensitive(
                GTK_WIDGET(gummi->gui->prefsgui->autosave_timer), FALSE);
        iofunctions_stop_autosave();
    }
}

void on_prefs_close_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    GtkTextIter start, end;
    if (2 == gtk_notebook_get_current_page(gummi->gui->prefsgui->notebook)) {
        gtk_text_buffer_get_start_iter(gummi->gui->prefsgui->default_buffer,
                &start);
        gtk_text_buffer_get_end_iter(gummi->gui->prefsgui->default_buffer,
                &end);
        config_set_value("welcome", gtk_text_buffer_get_text(
                    gummi->gui->prefsgui->default_buffer, &start, &end, FALSE));
    }
    gtk_widget_hide(GTK_WIDGET(gummi->gui->prefsgui->prefwindow));
}

void on_prefs_reset_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    config_set_default();
    prefsgui_set_current_settings(gummi->gui->prefsgui);
}

void on_tabwidth_value_changed(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar* val_str = g_strdup_printf("%d", newval);

    config_set_value("tabwidth", val_str);
    gtk_source_view_set_tab_width(gummi->editor->sourceview, newval);
    g_free(val_str);
}

void on_autosave_value_changed(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar* val_str = g_strdup_printf("%d", newval);

    config_set_value("autosave_timer", val_str);
    iofunctions_reset_autosave(gummi->finfo->filename);
    g_free(val_str);
}

void on_compile_value_changed(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar* val_str = g_strdup_printf("%d", newval);

    config_set_value("compile_timer", val_str);
    if (config_get_value("compile_status")) {
        motion_stop_updatepreview(gummi->motion);
        motion_start_updatepreview(gummi->motion);
    }
    g_free(val_str);
}

void on_editor_font_set(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    const gchar* font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(widget));
    slog(L_INFO, "setting font to %s\n", font);
    config_set_value("font", font);
    PangoFontDescription* font_desc = pango_font_description_from_string(font);
    gtk_widget_modify_font(GTK_WIDGET(gummi->editor->sourceview), font_desc);
    pango_font_description_free(font_desc);
}

void on_combo_typesetter_changed(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    const gchar typesetter[][16] = { "pdflatex", "xelatex" };
    if (0 == strcmp(config_get_value("typesetter"), "xelatex") &&
            1 == selected) return;
    config_set_value("typesetter", typesetter[selected]);
    gtk_widget_show(GTK_WIDGET(gummi->gui->prefsgui->changeimg));
    gtk_widget_show(GTK_WIDGET(gummi->gui->prefsgui->changelabel));
}

void on_combo_language_changed(GtkWidget* widget, void* user) {
    L_F_DEBUG;
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
    L_F_DEBUG;
    gint selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    const gchar scheme[][16] = { "on_idle", "real_time" };
    slog(L_INFO, "compile scheme set to %s\n", scheme[selected]);
    if (config_get_value("compile_status")) {
        motion_stop_updatepreview(gummi->motion);
        config_set_value("compile_scheme", scheme[selected]);
        motion_start_updatepreview(gummi->motion);
    } else {
        config_set_value("compile_scheme", scheme[selected]);
    }
}
