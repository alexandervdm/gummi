/**
 * @file    gui.c
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

#include "gui.h"

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

/* Many of the functions in this file are based on the excellent GTK+
 * tutorials written by Micah Carrick that can be found on: 
 * http://www.micahcarrick.com/gtk-glade-tutorial-part-3.html */

GummiGui* gui_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GtkWidget *hpaned;
    GtkWidget *errortext;
    gint width = 0, height = 0;

    GummiGui* g = (GummiGui*)g_malloc(sizeof(GummiGui));

    errortext = GTK_WIDGET(gtk_builder_get_object(builder, "errorfield"));
    g->mainwindow =
        GTK_WIDGET(gtk_builder_get_object(builder, "mainwindow"));
    g->toolbar =
        GTK_HBOX(gtk_builder_get_object(builder, "toolbar"));
    g->statusbar =
        GTK_STATUSBAR(gtk_builder_get_object(builder, "statusbar"));
    g->rightpane =
        GTK_VBOX(gtk_builder_get_object(builder, "rightpanebox"));
    g->previewoff = GTK_TOGGLE_TOOL_BUTTON(
            gtk_builder_get_object(builder, "tool_previewoff"));
    g->errorbuff =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(errortext));
    g->menu_spelling =
        GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "menu_spelling"));
    g->menu_toolbar =
        GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "menu_toolbar"));
    g->menu_statusbar =
        GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "menu_statusbar"));
    g->menu_rightpane =
        GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "menu_rightpane"));
    g->statusid =
        gtk_statusbar_get_context_id(GTK_STATUSBAR(g->statusbar), "Gummi");
    g->recent[0] =
        GTK_MENU_ITEM(gtk_builder_get_object(builder, "menu_recent1"));
    g->recent[1] =
        GTK_MENU_ITEM(gtk_builder_get_object(builder, "menu_recent2"));
    g->recent[2] =
        GTK_MENU_ITEM(gtk_builder_get_object(builder, "menu_recent3"));
    g->recent[3] =
        GTK_MENU_ITEM(gtk_builder_get_object(builder, "menu_recent4"));
    g->recent[4] =
        GTK_MENU_ITEM(gtk_builder_get_object(builder, "menu_recent5"));

    g->prefsgui = prefsgui_init(g);
    g->searchgui = searchgui_init(builder);
    g->importgui = importgui_init(builder);

    PangoFontDescription* font_desc = 
        pango_font_description_from_string("Monospace 8");
    gtk_widget_modify_font(errortext, font_desc);
    pango_font_description_free(font_desc);
    gtk_window_get_size(GTK_WINDOW(g->mainwindow), &width, &height);

    hpaned= GTK_WIDGET(gtk_builder_get_object(builder, "hpaned"));
    gtk_paned_set_position(GTK_PANED(hpaned), (width/2)); 

#ifndef USE_GTKSPELL
    gtk_widget_set_sensitive(GTK_WIDGET(g->menu_spelling), FALSE);
#else
    if (config_get_value("spelling"))
        gtk_check_menu_item_set_active(g->menu_spelling, TRUE);
#endif
    if (config_get_value("toolbar")) {
        gtk_check_menu_item_set_active(g->menu_toolbar, TRUE);
        gtk_widget_show(GTK_WIDGET(g->toolbar));
    }

    if (config_get_value("statusbar")) {
        gtk_check_menu_item_set_active(g->menu_statusbar, TRUE);
        gtk_widget_show(GTK_WIDGET(g->statusbar));
    }

    if (config_get_value("rightpane")) {
        gtk_check_menu_item_set_active(g->menu_rightpane, TRUE);
        gtk_widget_show(GTK_WIDGET(g->rightpane));
    } else {
        config_set_value("compile_status", "False");
        gtk_toggle_tool_button_set_active(g->previewoff, TRUE);
    }

    if (!config_get_value("compile_status"))
        gtk_toggle_tool_button_set_active(g->previewoff, TRUE);

    g->recent_list[0] = g_strdup(config_get_value("recent1"));
    g->recent_list[1] = g_strdup(config_get_value("recent2"));
    g->recent_list[2] = g_strdup(config_get_value("recent3"));
    g->recent_list[3] = g_strdup(config_get_value("recent4"));
    g->recent_list[4] = g_strdup(config_get_value("recent5"));

    display_recent_files(g);

    return g;
}

void gui_main(GtkBuilder* builder) {
    L_F_DEBUG;
    gtk_builder_connect_signals(builder, NULL);       
    g_signal_connect(g_e_buffer, "changed",
            G_CALLBACK(check_motion_timer), NULL);
    gtk_widget_show_all(gummi->gui->mainwindow);
    gtk_main();
}

gboolean gui_quit(void) {
    L_F_DEBUG;
    gint ret = check_for_save();
    if (GTK_RESPONSE_YES == ret)
        on_menu_save_activate(NULL, NULL);  
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return TRUE;
    gtk_main_quit();

    printf("   ___ \n"
            "  {o,o}    Thanks for using Gummi!\n" 
            "  |)__)    I welcome your feedback at:\n"
            "  -\"-\"-    http://gummi.midnightcoding.org\n\n");
    return FALSE;
}

void gui_update_title(void) {
    L_F_DEBUG;
    gchar* basename = NULL;
    gchar* dirname = NULL;
    gchar* title = NULL;
    if (gummi->finfo->filename) {
        basename = g_path_get_basename(gummi->finfo->filename);
        dirname = g_path_get_dirname(gummi->finfo->filename);
        title = g_strdup_printf("%s%s (%s) - %s",
                (gtk_text_buffer_get_modified(g_e_buffer)? "*": ""),
                basename, dirname, PACKAGE_NAME);
        g_free(basename);
        g_free(dirname);
    } else
        title = g_strdup_printf("%sUnsaved Document - %s",
                (gtk_text_buffer_get_modified(g_e_buffer)? "*": ""),
                PACKAGE_NAME);

    gtk_window_set_title(GTK_WINDOW(gummi->gui->mainwindow), title);
    g_free(title);
}

void on_menu_new_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gint ret = check_for_save();
    if (GTK_RESPONSE_YES == ret)
        on_menu_save_activate(NULL, NULL);  
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;
    iofunctions_load_default_text(gummi->editor);
    gummi_create_environment(gummi, NULL);
}

void on_menu_template_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    gtk_list_store_clear(gummi->templ->list_templates);
    template_setup(gummi->templ);
    gtk_widget_show_all(GTK_WIDGET(gummi->templ->templatewindow));
}

void on_menu_exportpdf_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    gchar* filename = get_save_filename(FILTER_PDF);
    if (filename)
        motion_export_pdffile(gummi->motion, filename);
}

void on_menu_recent_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    const gchar* name = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
    gchar* ptr;
    gint index = name[0] - '0' -1;
    gint ret = check_for_save();

    if (GTK_RESPONSE_YES == ret)
        on_menu_save_activate(NULL, NULL);  
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;

    if (utils_path_exists(gummi->gui->recent_list[index])) {
        iofunctions_load_file(gummi->editor, gummi->gui->recent_list[index]); 
        gummi_create_environment(gummi, gummi->gui->recent_list[index]);
    } else {
        ptr = g_strdup_printf(_("Error loading recent file: %s"),
                gummi->gui->recent_list[index]);
        statusbar_set_message(ptr);
        g_free(ptr);
        g_free(gummi->gui->recent_list[index]);
        while (index < 4) {
            gummi->gui->recent_list[index] = gummi->gui->recent_list[index+1];
            ++index;
        }
        gummi->gui->recent_list[4] = 0;
    }
    display_recent_files(gummi->gui);
}

void on_menu_open_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gchar *filename = NULL;
    gint ret = check_for_save();

    if (GTK_RESPONSE_YES == ret)
        on_menu_save_activate(NULL, NULL);  
    else if (GTK_RESPONSE_CANCEL == ret || GTK_RESPONSE_DELETE_EVENT == ret)
        return;
    filename = get_open_filename(FILTER_LATEX);
    if (filename != NULL) {
        iofunctions_load_file(gummi->editor, filename); 
        gummi_create_environment(gummi, filename);
        add_to_recent_list(filename);
    }
}

void on_menu_save_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gchar* filename = NULL;
    gboolean new = FALSE;

    if (!gummi->finfo->filename) {
        if ((filename = get_save_filename(FILTER_LATEX))) {
            if (strcmp(filename + strlen(filename) -4, ".tex")) {
                filename = g_strdup_printf("%s.tex", filename);
                new = TRUE;
            }
            fileinfo_set_filename(gummi->finfo, filename);
            iofunctions_write_file(gummi->editor, filename); 
            add_to_recent_list(filename);
            if (new) g_free(filename);
        }
    } else
        iofunctions_write_file(gummi->editor, gummi->finfo->filename); 
    gui_update_title();
}

void on_menu_saveas_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gchar* filename = NULL;
    gboolean new = FALSE;
    if ((filename = get_save_filename(FILTER_LATEX))) {
        if (strcmp(filename + strlen(filename) -4, ".tex")) {
            filename = g_strdup_printf("%s.tex", filename);
            new = TRUE;
        }
        iofunctions_write_file(gummi->editor, filename); 
        gummi_create_environment(gummi, filename);
        add_to_recent_list(filename);
        if (new) g_free(filename);
    }
}

void on_menu_cut_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard(g_e_buffer, clipboard, TRUE);
}

void on_menu_copy_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard(g_e_buffer, clipboard);
}
void on_menu_paste_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    GtkClipboard     *clipboard;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard(g_e_buffer, clipboard, NULL, TRUE);
}

void on_menu_undo_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    editor_undo_change(gummi->editor);
}

void on_menu_redo_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    editor_redo_change(gummi->editor);
}

void on_menu_delete_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    gtk_text_buffer_delete_selection(g_e_buffer, FALSE, TRUE);
}

void on_menu_selectall_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(g_e_buffer, &start, &end);
    gtk_text_buffer_select_range(g_e_buffer, &start, &end);
}

void on_menu_preferences_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    prefsgui_main();
}

void on_menu_statusbar_toggled(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
        gtk_widget_show(GTK_WIDGET(gummi->gui->statusbar));
        config_set_value("statusbar", "True");
    } else {
        gtk_widget_hide(GTK_WIDGET(gummi->gui->statusbar));
        config_set_value("statusbar", "False");
    }
}

void on_menu_toolbar_toggled(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
        gtk_widget_show(GTK_WIDGET(gummi->gui->toolbar));
        config_set_value("toolbar", "True");
    } else {
        gtk_widget_hide(GTK_WIDGET(gummi->gui->toolbar));
        config_set_value("toolbar", "False");
    }
}

void on_menu_rightpane_toggled(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
        gtk_widget_show(GTK_WIDGET(gummi->gui->rightpane));
        config_set_value("rightpane", "True");
    } else {
        gtk_widget_hide(GTK_WIDGET(gummi->gui->rightpane));
        config_set_value("rightpane", "False");
    }
}

    void on_menu_fullscreen_toggled(GtkWidget *widget, void * user) {
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
            gtk_window_fullscreen(GTK_WINDOW(gummi->gui->mainwindow));
        else
            gtk_window_unfullscreen(GTK_WINDOW(gummi->gui->mainwindow));
    }

void on_menu_find_activate(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gtk_entry_set_text(gummi->gui->searchgui->searchentry, "");
    gtk_entry_set_text(gummi->gui->searchgui->replaceentry, "");
    gtk_widget_grab_focus(GTK_WIDGET(gummi->gui->searchgui->searchentry));
    gtk_widget_show_all(GTK_WIDGET(gummi->gui->searchgui->searchwindow));
}

void on_menu_findnext_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    editor_jumpto_search_result(gummi->editor, 1);
}

void on_menu_findprev_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    editor_jumpto_search_result(gummi->editor, -1);
}

void on_menu_bibload_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    gchar *filename = NULL;
    filename = get_open_filename(FILTER_BIBLIO);
    if (biblio_check_valid_file(gummi->biblio, filename)) {
        biblio_setup_bibliography(gummi->biblio, gummi->editor);
        gtk_label_set_text(gummi->biblio->filenm_label,gummi->biblio->basename);
    }
}

void on_menu_bibupdate_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    biblio_compile_bibliography(gummi->biblio, gummi->motion);
}

void on_menu_docstat_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    GtkWidget* dialog = 0;
    gint i = 0;
    gchar* str = 0;
    gchar* output = 0;
    gchar* cmd = g_strdup_printf("texcount %s", gummi->finfo->workfile);
    pdata result = utils_popen_r(cmd);
    gchar* terms[] = { _("Words in text"),
                       _("Words in headers"),
                       _("Words in float captions"),
                       _("Number of headers"),
                       _("Number of floats"),
                       _("Number of math inlines"),
                       _("Number of math displayed")
                     };
    gchar* res[7];

    str = strtok(result.data, ":\n");
    while (str) {
      str = strtok(NULL, ":\n");
      if (strlen(str) > 10) continue;
      res[i++] = str;
      if (i == 7) break;
    }

    output = g_strconcat(terms[0], ": ", res[0], "\n",
                         terms[1], ": ", res[1], "\n",
                         terms[2], ": ", res[2], "\n",
                         terms[3], ": ", res[3], "\n",
                         terms[4], ": ", res[4], "\n",
                         terms[4], ": ", res[5], "\n",
                         terms[6], ": ", res[6], "\n",
                         NULL);

    dialog = gtk_message_dialog_new(GTK_WINDOW(gummi->gui->mainwindow),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "%s", output);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Document Statistic"));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(cmd);
    g_free(output);
}

void on_menu_spelling_toggled(GtkWidget *widget, void * user) {
    L_F_DEBUG;
#ifdef USE_GTKSPELL
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
        editor_activate_spellchecking(gummi->editor, TRUE);
        config_set_value("spelling", "True");
    } else {
        editor_activate_spellchecking(gummi->editor, FALSE);
        config_set_value("spelling", "False");
    }
#endif
}

void on_menu_update_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    gboolean ret = updatecheck(GTK_WINDOW(gummi->gui->mainwindow));
    if (!ret)
        slog(L_G_ERROR, "Update check failed!\n");
}

void on_menu_about_activate(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    GError* err = NULL;
    GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_size
        (DATADIR"/gummi-beta.png", 60, 60, &err);
    const gchar* authors[] = { "Alexander van der Mey\n"
        "<alexvandermey@gmail.com>",
        "Wei-Ning Huang\n"
            "<aitjcize@gmail.com>\n",
        "Contributors:",
        "Thomas van der Burgt",
        "Cameron Grout", NULL };
    const gchar* artists[] = {"Windows version Icon set from Elemetary Project:\n"
        "http://www.elementary-project.com/", NULL};
                
    const gchar* translators =
        "Brazilian-Portugese: Fernando Cruz\n"
        "Catalan: Marc Vinyals\n"
        "Danish: Jack Olsen\n"
        "Dutch: Alexander van der Mey\n"
        "French: Yvan Duron & Olivier Brousse\n"
        "German: Thomas Niederprüm\n"
        "Greek: Dimitris Leventeas\n"
        "Italian: Salvatore Vassallo\n" 
        "Russian: Kruvalig\n"
        "Taiwanese: Wei-Ning Huang\n";

    GtkAboutDialog* dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_window_set_transient_for(GTK_WINDOW(dialog),
            GTK_WINDOW(gummi->gui->mainwindow));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
    gtk_about_dialog_set_authors(dialog, authors);
    gtk_about_dialog_set_program_name(dialog, PACKAGE_NAME);
    gtk_about_dialog_set_version(dialog, PACKAGE_VERSION);
    gtk_about_dialog_set_website(dialog, PACKAGE_URL);
    gtk_about_dialog_set_copyright(dialog, PACKAGE_COPYRIGHT);
    gtk_about_dialog_set_license(dialog, PACKAGE_LICENSE);
    gtk_about_dialog_set_logo(dialog, icon);
    gtk_about_dialog_set_comments(dialog, PACKAGE_COMMENTS);
    gtk_about_dialog_set_artists(dialog, artists);
    gtk_about_dialog_set_translator_credits(dialog, translators);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void on_tool_previewoff_toggled(GtkWidget *widget, void * user) {
    L_F_DEBUG;
    gboolean value =
        gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget));
    config_set_value("compile_status", (!value)?"Ture":"False");
    if (value)
        motion_stop_updatepreview(gummi->motion);
    else
        motion_start_updatepreview(gummi->motion);
}

void on_tool_textstyle_bold_activate(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_set_selection_textstyle(gummi->editor, "tool_bold");
}

void on_tool_textstyle_italic_activate(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_set_selection_textstyle(gummi->editor, "tool_italic");
}

void on_tool_textstyle_underline_activate(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_set_selection_textstyle(gummi->editor, "tool_unline");
}

void on_tool_textstyle_left_activate(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_set_selection_textstyle(gummi->editor, "tool_left");
}

void on_tool_textstyle_center_activate(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_set_selection_textstyle(gummi->editor, "tool_center");
}

void on_tool_textstyle_right_activate(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_set_selection_textstyle(gummi->editor, "tool_right");
}



void on_button_template_add_clicked(GtkWidget* widget, void* user) {
    gchar *doc = editor_grab_buffer(gummi->editor);
    template_add_new_entry(gummi->templ, doc);
}

void on_button_template_remove_clicked(GtkWidget* widget, void* user) {
    template_remove_entry(gummi->templ);
}

void on_button_template_open_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gchar *text;
    text = template_open_selected(gummi->templ);
    if (text) {
        editor_fill_buffer(gummi->editor, text);
        gummi_create_environment(gummi, NULL);
        gtk_widget_hide(GTK_WIDGET(gummi->templ->templatewindow));
    }
}

void on_button_template_close_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gtk_widget_hide(GTK_WIDGET(gummi->templ->templatewindow));
}

void on_template_rowitem_editted(GtkWidget* widget, gchar *path, gchar* text, gpointer data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    
    model = gtk_tree_view_get_model(gummi->templ->templateview);
    selection = gtk_tree_view_get_selection(gummi->templ->templateview);
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_list_store_set(gummi->templ->list_templates, &iter, 0, text, -1);
    }
    printf("TODO: move or create template file\n");
}


gboolean on_button_searchwindow_close_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gtk_widget_hide(GTK_WIDGET(gummi->gui->searchgui->searchwindow));
    return TRUE;
}

void on_button_searchwindow_find_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_start_search(gummi->editor,
            gtk_entry_get_text(gummi->gui->searchgui->searchentry),
            gummi->gui->searchgui->backwards,
            gummi->gui->searchgui->wholeword,
            gummi->gui->searchgui->matchcase,
            0
            );
}

void on_button_searchwindow_replace_next_clicked(GtkWidget* widget, void* user)
{
    editor_start_replace_next(gummi->editor,
            gtk_entry_get_text(gummi->gui->searchgui->searchentry),
            gtk_entry_get_text(gummi->gui->searchgui->replaceentry),
            gummi->gui->searchgui->backwards,
            gummi->gui->searchgui->wholeword,
            gummi->gui->searchgui->matchcase
            );
}

void on_button_searchwindow_replace_all_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    editor_start_replace_all(gummi->editor,
            gtk_entry_get_text(gummi->gui->searchgui->searchentry),
            gtk_entry_get_text(gummi->gui->searchgui->replaceentry),
            gummi->gui->searchgui->backwards,
            gummi->gui->searchgui->wholeword,
            gummi->gui->searchgui->matchcase
            );
}

GuImportGui* importgui_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GuImportGui* i = (GuImportGui*)g_malloc(sizeof(GuImportGui));
    i->box_image =
        GTK_HBOX(gtk_builder_get_object(builder, "box_image"));
    i->box_table =
        GTK_HBOX(gtk_builder_get_object(builder, "box_table"));
    i->box_matrix =
        GTK_HBOX(gtk_builder_get_object(builder, "box_matrix"));
    i->image_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "image_pane"));
    i->table_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "table_pane"));
    i->matrix_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "matrix_pane"));
    return i;
}

void on_button_import_table_apply_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    importer_insert_table(gummi->importer, gummi->editor);
}

void on_button_import_image_apply_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    importer_insert_image(gummi->importer, gummi->editor);
}

void on_button_import_matrix_apply_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    importer_insert_matrix(gummi->importer, gummi->editor);
}

void on_image_file_activate(void) {
    L_F_DEBUG;
    const gchar* filename = get_open_filename(FILTER_IMAGE);
    importer_imagegui_set_sensitive(gummi->importer, filename, TRUE);
}

void on_import_tabs_switch_page(GtkNotebook* notebook, GtkNotebookPage* page,
        guint page_num, void* user) {
    GList* list = NULL;
    list = gtk_container_get_children(
            GTK_CONTAINER(gummi->gui->importgui->box_image));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(gummi->gui->importgui->box_image),
                GTK_WIDGET(list->data));
        list = list->next;
    }
    list = gtk_container_get_children(
            GTK_CONTAINER(gummi->gui->importgui->box_table));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(gummi->gui->importgui->box_table),
                GTK_WIDGET(list->data));
        list = list->next;
    }
    list = gtk_container_get_children(
            GTK_CONTAINER(gummi->gui->importgui->box_matrix));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(gummi->gui->importgui->box_matrix),
                GTK_WIDGET(list->data));
        list = list->next;
    }

    switch (page_num) {
        case 1:
            gtk_container_add(GTK_CONTAINER(gummi->gui->importgui->box_image),
                    GTK_WIDGET(gummi->gui->importgui->image_pane));
            break;
        case 2:
            gtk_container_add(GTK_CONTAINER(gummi->gui->importgui->box_table),
                    GTK_WIDGET(gummi->gui->importgui->table_pane));
            break;
        case 3:
            gtk_container_add(GTK_CONTAINER(gummi->gui->importgui->box_matrix),
                    GTK_WIDGET(gummi->gui->importgui->matrix_pane));
            break;
    }
}

void on_bibcolumn_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint id = gtk_tree_view_column_get_sort_column_id
        (GTK_TREE_VIEW_COLUMN(widget));
    gtk_tree_view_column_set_sort_column_id
        (GTK_TREE_VIEW_COLUMN(widget), id);
}

void on_bibcompile_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gummi->biblio->progressval = 0.0;
    g_timeout_add(10, on_bibprogressbar_update, NULL);

    if (biblio_compile_bibliography(gummi->biblio, gummi->motion)) {
        statusbar_set_message(_("Compiling bibliography file..."));
        gtk_progress_bar_set_text(gummi->biblio->progressbar,
                _("bibliography compiled without errors"));
    } else {
        statusbar_set_message(_("Error compiling bibliography file or none "
                    "detected..."));
        gtk_progress_bar_set_text(gummi->biblio->progressbar,
                _("error compiling bibliography file"));
    }
    check_motion_timer();
}

void on_bibrefresh_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gchar* text = 0;
    gchar* str = 0;
    GError* err = NULL;
    gint number = 0;

    gummi->biblio->progressval = 0.0;
    g_timeout_add(2, on_bibprogressbar_update, NULL);
    gtk_list_store_clear(gummi->biblio->list_biblios);

    if (biblio_detect_bibliography(gummi->editor)) {
        biblio_setup_bibliography(gummi->biblio, gummi->editor);
        g_file_get_contents(gummi->biblio->filename, &text, NULL, &err);
        number = biblio_parse_entries(gummi->biblio, text);
        gtk_label_set_text(gummi->biblio->filenm_label,
                gummi->biblio->basename);
        str = g_strdup_printf("%d", number);
        gtk_label_set_text(gummi->biblio->refnr_label, str);
        g_free(str);
        str = g_strdup_printf(_("%s loaded"), gummi->biblio->basename);
        gtk_progress_bar_set_text(gummi->biblio->progressbar, str);
        g_free(str);
    }
    else {
        gtk_progress_bar_set_text(gummi->biblio->progressbar,
                _("no bibliography file detected"));
        gtk_label_set_text(gummi->biblio->filenm_label, _("none"));
        gtk_label_set_text(gummi->biblio->refnr_label, _("N/A"));
    }
}


void on_bibreference_clicked(GtkTreeView* view, GtkTreePath* Path,
        GtkTreeViewColumn* column, void* user) {
    GtkTreeIter iter;
    gchar* value;
    gchar* out;
    GtkTreeModel* model = GTK_TREE_MODEL(gummi->biblio->list_biblios);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(view);

    gtk_tree_selection_get_selected(selection, &model, &iter);
    gtk_tree_model_get(model, &iter, 0, &value, -1);
    out = g_strdup_printf("\\cite{%s}", value);
    gtk_text_buffer_insert_at_cursor(g_e_buffer, out, strlen(out));
    g_free(out);
}

gboolean on_bibprogressbar_update(void* user) {
    L_F_DEBUG;
    gtk_adjustment_set_value
        (gummi->biblio->progressmon, gummi->biblio->progressval);
    gummi->biblio->progressval += 1.0;
    if (gummi->biblio->progressval > 60) return FALSE;
    else return TRUE;
}


void preview_next_page(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    preview_goto_page(gummi->preview, gummi->preview->page_current + 1);
}

void preview_prev_page(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    preview_goto_page(gummi->preview, gummi->preview->page_current - 1);
}

void preview_zoom_change(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    double opts[9] = {0.50, 0.70, 0.85, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0}; 

    if (index < 0) slog(L_ERROR, "preview zoom level is < 0.\n");

    gummi->preview->fit_width = gummi->preview->best_fit = FALSE;
    if (index < 2) {
        if (index == 0) {
            gummi->preview->best_fit = TRUE;
        }
        else if (index == 1) {
            gummi->preview->fit_width = TRUE;
        }
    }
    else {
        gummi->preview->page_scale = opts[index-2];
    }

    gtk_widget_queue_draw(gummi->preview->drawarea);
}

GuPrefsGui* prefsgui_init(GummiGui* gui) {
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
            GTK_WINDOW(gui->mainwindow));

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
            atoi(config_get_value("autosave_timer"))/60);
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
        gtk_text_view_set_wrap_mode(g_e_view, GTK_WRAP_NONE);
        config_set_value("wordwrapping", "False");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                    gummi->gui->prefsgui->wordwrap_button), FALSE);
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
        gtk_spin_button_set_value(gummi->gui->prefsgui->autosave_timer,
                time / 60);
        iofunctions_start_autosave(time, gummi->finfo->filename);
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
    gchar val_str[16];

    snprintf(val_str, 16, "%d", newval);
    config_set_value("tabwidth", val_str);
    gtk_source_view_set_tab_width(gummi->editor->sourceview, newval);
}

void on_autosave_value_changed(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar val_str[16];

    snprintf(val_str, 16, "%d", newval);
    config_set_value("autosave_timer", val_str);
    iofunctions_reset_autosave(gummi->finfo->filename);
}

void on_compile_value_changed(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    gint newval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
    gchar val_str[16];

    snprintf(val_str, 16, "%d", newval);
    config_set_value("compile_timer", val_str);
    if (config_get_value("compile_status")) {
        motion_stop_updatepreview(gummi->motion);
        motion_start_updatepreview(gummi->motion);
    }
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

GuSearchGui* searchgui_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GuSearchGui* s;
    s = (GuSearchGui*)g_malloc(sizeof(GuSearchGui));
    s->searchwindow =
        GTK_WIDGET(gtk_builder_get_object(builder, "searchwindow"));
    s->searchentry =
        GTK_ENTRY(gtk_builder_get_object(builder, "searchentry"));
    s->replaceentry =
        GTK_ENTRY(gtk_builder_get_object(builder, "replaceentry"));
    s->matchcase = TRUE;
    g_signal_connect(s->searchentry, "changed",
            G_CALLBACK(on_searchgui_text_changed), NULL);
    s->backwards = FALSE;
    s->wholeword = FALSE;
    return s;
}

void on_toggle_matchcase_toggled(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gummi->gui->searchgui->matchcase =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gummi->editor->replace_activated = FALSE;
}

void on_toggle_wholeword_toggled(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gummi->gui->searchgui->wholeword =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gummi->editor->replace_activated = FALSE;
}

void on_toggle_backwards_toggled(GtkWidget *widget, void* user) {
    L_F_DEBUG;
    gummi->gui->searchgui->backwards =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gummi->editor->replace_activated = FALSE;
}

void on_searchgui_text_changed(GtkEditable *editable, void* user) {
    L_F_DEBUG;
    gummi->editor->replace_activated = FALSE;
}

gint check_for_save(void) {
    L_F_DEBUG;
    gint ret = 0;

    if (gtk_text_buffer_get_modified(g_e_buffer))
        ret = utils_yes_no_dialog(
                _("Do you want to save the changes you have made?"));
    return ret;
}

gchar* get_open_filename(GuFilterType type) {
    L_F_DEBUG;
    GtkFileChooser* chooser = NULL;
    gchar* filename = NULL;

    chooser = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new(
                _("Open File..."),
                GTK_WINDOW (gummi->gui->mainwindow),
                GTK_FILE_CHOOSER_ACTION_OPEN,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                NULL));

    file_dialog_set_filter(chooser, type);
    gtk_file_chooser_set_current_folder(chooser, g_get_home_dir());

    if (gtk_dialog_run(GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

    gtk_widget_destroy(GTK_WIDGET(chooser));
    return filename;
}

gchar* get_save_filename(GuFilterType type) {
    L_F_DEBUG;
    GtkFileChooser* chooser = NULL;
    gchar* filename = NULL;

    chooser = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new(
                _("Save File..."),
                GTK_WINDOW (gummi->gui->mainwindow),
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_OK,
                NULL));

    file_dialog_set_filter(chooser, type);
    gtk_file_chooser_set_current_folder(chooser, g_get_home_dir());

    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

    gtk_widget_destroy(GTK_WIDGET(chooser));
    return filename;
}

void file_dialog_set_filter(GtkFileChooser* dialog, GuFilterType type) {
    L_F_DEBUG;
    GtkFileFilter* filter = gtk_file_filter_new();

    switch (type) {
        case FILTER_LATEX:
            gtk_file_filter_set_name(filter, "LaTeX files");
            gtk_file_filter_add_pattern(filter, "*.tex");
            gtk_file_chooser_add_filter(dialog, filter);
            gtk_file_chooser_set_filter(dialog, filter);
            filter = gtk_file_filter_new();
            gtk_file_filter_set_name(filter, "Text files");
            gtk_file_filter_add_mime_type(filter, "text/plain");
            gtk_file_chooser_add_filter(dialog, filter);
            break;

        case FILTER_PDF:
            gtk_file_filter_set_name(filter, "PDF files");
            gtk_file_filter_add_pattern(filter, "*.pdf");
            gtk_file_chooser_add_filter(dialog, filter);
            gtk_file_chooser_set_filter(dialog, filter);
            break;

        case FILTER_IMAGE:
            gtk_file_filter_set_name(filter, "Image files");
            gtk_file_filter_add_mime_type(filter, "image/*");
            gtk_file_chooser_add_filter(dialog, filter);
            gtk_file_chooser_set_filter(dialog, filter);
            break;

        case FILTER_BIBLIO:
            gtk_file_filter_set_name(filter, "Bibtex files");
            gtk_file_filter_add_pattern(filter, "*.bib");
            gtk_file_chooser_add_filter(dialog, filter);
            gtk_file_chooser_set_filter(dialog, filter);
            break;
    }

}

void add_to_recent_list(gchar* filename) {
    L_F_DEBUG;
    gint i = 0;
    /* add to recent list */
    g_free(gummi->gui->recent_list[4]);
    for (i = 3; i >= 0; --i)
        gummi->gui->recent_list[i + 1] = gummi->gui->recent_list[i];
    gummi->gui->recent_list[0] = g_strdup(filename);
    display_recent_files(gummi->gui);
}

void display_recent_files(GummiGui* gui) {
    L_F_DEBUG;
    gchar* ptr = 0;
    gint i = 0, count = 0;

    for (i = 0; i < 5; ++i)
        gtk_widget_hide(GTK_WIDGET(gui->recent[i]));

    for (i = 0; i < 5; ++i) {
        if (gui->recent_list[i] &&
            0 != strcmp(gui->recent_list[i], "__NULL__")) {
            ptr = g_strdup_printf("%d. %s", count + 1,
                    g_path_get_basename(gui->recent_list[i]));
            gtk_menu_item_set_label(gui->recent[i], ptr);
            gtk_widget_show(GTK_WIDGET(gui->recent[i]));
            g_free(ptr);
            ++count;
        }
    }
    /* update configuration file */
    for (i = 0; i < 5; ++i) {
        ptr = g_strdup_printf("recent%d", i + 1);
        if (gui->recent_list[i])
            config_set_value(ptr, gui->recent_list[i]);
        else
            config_set_value(ptr, "__NULL__");
        g_free(ptr);
    }
}

void errorbuffer_set_text(gchar *message) {
    L_F_DEBUG;
    gtk_text_buffer_set_text(gummi->gui->errorbuff, message, -1);
}

void statusbar_set_message(gchar *message) {
    L_F_DEBUG;
    gtk_statusbar_push (GTK_STATUSBAR(gummi->gui->statusbar),
            gummi->gui->statusid, message);
    g_timeout_add_seconds(4, statusbar_del_message, NULL);
}

gboolean statusbar_del_message(void* user) {
    L_F_DEBUG;
    gtk_statusbar_pop(GTK_STATUSBAR(gummi->gui->statusbar),
            gummi->gui->statusid);
    return FALSE;
}

void check_motion_timer(void) {
    L_F_DEBUG;
    gtk_text_buffer_set_modified(g_e_buffer, TRUE);
    gummi->editor->replace_activated = FALSE;
    gummi->motion->modified_since_compile = TRUE;
    gui_update_title();

    if (config_get_value("compile_status") &&
            0 == strcmp(config_get_value("compile_scheme"), "on_idle")) {
        motion_start_timer(gummi->motion);
    }
}
