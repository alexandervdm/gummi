/**
 * @file    editor.c
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


#include "editor.h"

#include <stdlib.h>
#include <string.h>

#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourceiter.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourceview.h>
#ifdef USE_GTKSPELL
#   include <gtkspell/gtkspell.h>
#endif
#include <gtk/gtk.h>

#include "configfile.h"
#include "environment.h"
#include "utils.h"

const gchar style[][3][20] = {
    { "tool_bold", "\\textbf{", "}" },
    { "tool_italic", "\\textit{", "}" },
    { "tool_unline", "\\underline{", "}" },
    { "tool_left", "\\begin{flushleft}", "\\end{flushleft}"},
    { "tool_center", "\\begin{flushcenter}", "\\end{flushcenter}"},
    { "tool_right", "\\begin{flushright}", "\\end{flushright}"}
};

/* reference to global environment instance */
extern Gummi* gummi;

GuEditor* editor_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GtkWidget *scroll;
    GtkSourceLanguageManager* manager = gtk_source_language_manager_new();
    GtkSourceLanguage* lang = gtk_source_language_manager_get_language
                                                        (manager, "latex");
    GuEditor* ec = (GuEditor*)g_malloc(sizeof(GuEditor));
    ec->sourcebuffer = gtk_source_buffer_new_with_language(lang);
    ec->sourceview =
        GTK_SOURCE_VIEW(gtk_source_view_new_with_buffer(ec->sourcebuffer));
    ec->errortag = gtk_text_tag_new("error");
    ec->searchtag = gtk_text_tag_new("search");
    ec->editortags = gtk_text_buffer_get_tag_table(ec_sourcebuffer);
    ec->replace_activated = FALSE;
    ec->term = NULL;
    ec->cur_swap = FALSE;
    
    scroll = GTK_WIDGET (gtk_builder_get_object (builder, "editor_scroll"));
    gtk_container_add (GTK_CONTAINER (scroll), GTK_WIDGET(ec->sourceview));

    gtk_source_view_set_tab_width(ec->sourceview,
            atoi(config_get_value("tabwidth")));
    gtk_source_view_set_insert_spaces_instead_of_tabs(ec->sourceview,
            (gboolean)config_get_value("tabs_instof_spaces"));
    gtk_source_view_set_auto_indent(ec->sourceview,
            (gboolean)config_get_value("autoindentation"));

#ifdef USE_GTKSPELL
    if (config_get_value("spelling"))
        editor_activate_spellchecking(ec, TRUE);
#endif

    editor_sourceview_config(ec);

    gtk_text_buffer_set_modified(ec_sourcebuffer, FALSE);
    return ec;
}

void editor_sourceview_config(GuEditor* ec) {
    L_F_DEBUG;
    GtkWrapMode wrapmode = 0;

    gtk_source_buffer_set_highlight_matching_brackets(ec->sourcebuffer, TRUE);

    const gchar* font = config_get_value("font");
    slog(L_INFO, "setting font to %s\n", font);
    PangoFontDescription* font_desc = pango_font_description_from_string(font);
    gtk_widget_modify_font(GTK_WIDGET(ec->sourceview), font_desc);
    pango_font_description_free(font_desc);

    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(ec->sourceview),
            (gboolean)config_get_value("line_numbers"));
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(ec->sourceview),
        (gboolean)config_get_value("highlighting"));

    if (config_get_value("textwrapping"))
        wrapmode += 1;
    if (config_get_value("wordwrapping"))
        wrapmode += 2;
    gtk_text_view_set_wrap_mode(ec_sourceview, wrapmode);
    g_object_set(G_OBJECT(ec->errortag), "background", "red",
            "foreground", "white", NULL);
    g_object_set(G_OBJECT(ec->searchtag), "background", "yellow", NULL);
}

#ifdef USE_GTKSPELL
void editor_activate_spellchecking(GuEditor* ec, gboolean status) {
    L_F_DEBUG;
    const gchar* lang = config_get_value("spell_language");
    GError* err = NULL;
    GError* err2 = NULL;
    if (status) {
        GtkSpell* spell = gtkspell_new_attach(ec_sourceview, NULL, &err);
        if (!spell)
            slog(L_INFO, "gtkspell: %s\n", err->message);
        if (!gtkspell_set_language(spell, lang, &err2))
            slog(L_INFO, "gtkspell: %s\n", err2->message);
    } else {
        GtkSpell* spell = gtkspell_get_from_text_view(ec_sourceview);
        if (spell)
            gtkspell_detach(spell);
    }
}
#endif

void editor_fill_buffer(GuEditor* ec, const gchar* text) {
    L_F_DEBUG;
    gtk_text_buffer_begin_user_action(ec_sourcebuffer);
    gtk_source_buffer_begin_not_undoable_action(ec->sourcebuffer);
    gtk_widget_set_sensitive(GTK_WIDGET(ec->sourceview), FALSE);
    gtk_text_buffer_set_text(ec_sourcebuffer, text, strlen(text));
    gtk_text_buffer_set_modified(ec_sourcebuffer, FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(ec->sourceview), TRUE);
    gtk_source_buffer_end_not_undoable_action(ec->sourcebuffer);
    gtk_text_buffer_end_user_action(ec_sourcebuffer);
}

gchar* editor_grab_buffer(GuEditor* ec) {
    L_F_DEBUG;
    GtkTextIter start, end;
    gtk_widget_set_sensitive(GTK_WIDGET(ec->sourceview), FALSE);
    gtk_text_buffer_get_bounds(ec_sourcebuffer, &start, &end);
    gtk_widget_set_sensitive(GTK_WIDGET(ec->sourceview), TRUE);
    gchar* pstr = gtk_text_iter_get_text (&start, &end);
    return pstr;
}

void editor_insert_package(GuEditor* ec, const gchar* package) {
    L_F_DEBUG;
    GtkTextIter start, mstart, mend, sstart, send;
    gchar pkgstr[BUFSIZ / 64];
    gtk_text_buffer_get_start_iter(ec_sourcebuffer, &start);
    gtk_text_iter_forward_search(&start, (gchar*)"\\begin{document}", 0,
            &mstart, &mend, NULL);
    snprintf(pkgstr, BUFSIZ / 64, "\\usepackage{%s}\n", package);
    if (!gtk_text_iter_backward_search(&mstart, pkgstr, 0, &sstart, &send,
                NULL)) {
        gtk_source_buffer_begin_not_undoable_action(ec->sourcebuffer);
        gtk_text_buffer_begin_user_action(ec_sourcebuffer);
        gtk_text_buffer_insert(ec_sourcebuffer, &mstart,
                pkgstr, strlen(pkgstr));
        gtk_text_buffer_end_user_action(ec_sourcebuffer);
        gtk_source_buffer_end_not_undoable_action(ec->sourcebuffer);
        gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
    }
}

void editor_insert_bib(GuEditor* ec, const gchar* package) {
    L_F_DEBUG;
    GtkTextIter start, end, mstart, mend, sstart, send;
    gchar pkgstr[BUFSIZ / 64];
    gtk_text_buffer_get_start_iter(ec_sourcebuffer, &start);
    gtk_text_buffer_get_end_iter(ec_sourcebuffer, &end);
    gtk_text_iter_backward_search(&end, (gchar*)"\\end{document}", 0,
            &mstart, &mend, NULL);
    snprintf(pkgstr, BUFSIZ / 64,
            "\\bibliography{%s}{}\n\\bibliographystyle{plain}\n", package);
    if (!gtk_text_iter_forward_search(&start, "\\bibliography{", 0,
                &sstart, &send, NULL)) {
        gtk_source_buffer_begin_not_undoable_action(ec->sourcebuffer);
        gtk_text_buffer_begin_user_action(ec_sourcebuffer);
        gtk_text_buffer_insert(ec_sourcebuffer, &mstart,
                pkgstr, strlen(pkgstr));
        gtk_text_buffer_end_user_action(ec_sourcebuffer);
        gtk_source_buffer_end_not_undoable_action(ec->sourcebuffer);
        gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
    }
}

void editor_set_selection_textstyle(GuEditor* ec, const gchar* type) {
    L_F_DEBUG;
    GtkTextIter start, end;
    gint i = 0, selected = 0, outsize = 0;
    const gchar* selected_text = 0;
    gint style_size = sizeof(style) / sizeof(style[0]);
    gchar** result = 0;
    GError* error = NULL;
    GRegex* match_str = 0;
    GMatchInfo* match_info;
    gchar* outtext = 0;
    gchar regexbuf[BUFSIZ];

    /* grab selected text */
    gtk_text_buffer_get_selection_bounds(ec_sourcebuffer, &start, &end);
    selected_text = gtk_text_iter_get_text(&start, &end);
    outsize = strlen(selected_text) + 64;
    outtext = (gchar*)g_malloc(outsize);

    /* select style */
    for (i = 0; i < style_size; ++i)
        if (0 == strcmp(style[i][0], type)) {
            selected = i;
            break;
        }

    /* generate regex expression */
    snprintf(regexbuf, BUFSIZ, "(.*)%s%s(.*)%s%s(.*)",
            (style[selected][1][0] == '\\')? "\\": "",
            style[selected][1],
            (style[selected][2][0] == '\\')? "\\": "",
            style[selected][2]);

    match_str = g_regex_new(regexbuf, G_REGEX_DOTALL, 0, &error);

    if (g_regex_match(match_str, selected_text, 0, &match_info)) {
        result = g_match_info_fetch_all(match_info);
        if (strlen(result[1]) == 0 && strlen(result[3]) == 0) {
            /* already applied, so we remove it */
            strncpy(outtext, result[2], outsize);
            outtext[strlen(result[2])] = 0;
        } else if (strlen(result[1]) != 0 || strlen(result[3]) != 0) {
            /* the text contains a partially styled text, remove it and apply
             * style to the whole text */
            snprintf(outtext, outsize, "%s%s%s%s%s",
                    style[selected][1], result[1], result[2], result[3],
                    style[selected][2]);
        }
    } else { /* no previous style applied */
        snprintf(outtext, outsize, "%s%s%s",
                style[selected][1], selected_text, style[selected][2]);
    }

    /* free memory */
    g_strfreev(result);
    g_match_info_free(match_info);
    g_regex_unref(match_str);

    gtk_text_buffer_begin_user_action(ec_sourcebuffer);
    gtk_text_buffer_delete(ec_sourcebuffer, &start, &end);
    gtk_text_buffer_insert(ec_sourcebuffer, &start, outtext, strlen(outtext));
    end = start;
    gtk_text_iter_backward_chars(&start, strlen(outtext));
    gtk_text_buffer_select_range(ec_sourcebuffer, &start, &end);
    gtk_text_buffer_end_user_action(ec_sourcebuffer);
    g_free(outtext);
    gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
}

void editor_apply_errortags(GuEditor* ec, gint line) {
    L_F_DEBUG;
    GtkTextIter start, end, error;
    /* remove the tag from the table if it is in threre */
    if (gtk_text_tag_table_lookup(ec->editortags, "error"))
        gtk_text_tag_table_remove(ec->editortags, ec->errortag);

    if (line) {
        gtk_text_tag_table_add(ec->editortags, ec->errortag);
        gtk_text_buffer_get_iter_at_line(ec_sourcebuffer, &start, line -1);
        gtk_text_buffer_get_iter_at_line(ec_sourcebuffer, &end, line);
        gtk_text_buffer_apply_tag(ec_sourcebuffer, ec->errortag, &start, &end);
        /* scroll to the error line without touching the curosr */
        gtk_text_buffer_get_iter_at_line(ec_sourcebuffer, &error, line);
        gtk_text_buffer_create_mark(ec_sourcebuffer, "error", &error, TRUE);
        gtk_text_view_scroll_to_mark(ec_sourceview, 
            gtk_text_buffer_get_mark(ec_sourcebuffer, "error"),
            0.25, FALSE, 0.0, 0.0);
    }
}

void editor_jumpto_search_result(GuEditor* ec, gint direction) {
    L_F_DEBUG;
    if (!ec->term) return;
    if (direction == 1)
        editor_search_next(ec, FALSE);
    else
        editor_search_next(ec, TRUE);
}

void editor_start_search(GuEditor* ec, const gchar* term,
        gboolean backwards, gboolean wholeword, gboolean matchcase,
        gboolean cs) {
    L_F_DEBUG;
    /* save options */
    if (ec->term != term) {
        if (ec->term) g_free(ec->term);
        ec->term = (gchar*)g_malloc(strlen(term) + 1);
        strcpy(ec->term, term);
    }

    ec->backwards = backwards;
    ec->wholeword = wholeword;
    ec->matchcase = matchcase;
    ec->cur_swap = cs;

    editor_apply_searchtag(ec);
    editor_search_next(ec, FALSE);
}

void editor_apply_searchtag(GuEditor* ec) {
    L_F_DEBUG;
    GtkTextIter start, mstart, mend;
    gboolean ret = FALSE;

    gtk_text_buffer_get_start_iter(ec_sourcebuffer, &start);

    if (gtk_text_tag_table_lookup(ec->editortags, "search"))
        gtk_text_tag_table_remove(ec->editortags, ec->searchtag);
    gtk_text_tag_table_add(ec->editortags, ec->searchtag);

    while (TRUE) {
        ret = gtk_source_iter_forward_search(&start, ec->term,
                (ec->matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);

        if (ret && (!ec->wholeword || (ec->wholeword
                        && gtk_text_iter_starts_word(&mstart)
                        && gtk_text_iter_ends_word(&mend)))) {
            gtk_text_buffer_apply_tag(ec_sourcebuffer, ec->searchtag,
                    &mstart, &mend);
            start =  mend;
        } else break;
    }
}

void editor_search_next(GuEditor* ec, gboolean inverse) {
    L_F_DEBUG;
    GtkTextIter current, start, end, mstart, mend;
    gboolean ret = FALSE, response = FALSE;

    /* place cursor to the next result */
    editor_get_current_iter(ec, &current);

    if (ec->backwards ^ inverse)
       ret = gtk_source_iter_backward_search(&current, ec->term,
                (ec->matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);
    else
       ret = gtk_source_iter_forward_search(&current, ec->term,
                (ec->matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);

    if (ret && (!ec->wholeword || (ec->wholeword
            && gtk_text_iter_starts_word(&mstart)
            && gtk_text_iter_ends_word(&mend)))) {
        gtk_text_buffer_place_cursor(ec_sourcebuffer,
                (ec->cur_swap ^ ec->backwards ^ inverse)? &mstart: &mend);
        editor_scroll_to_cursor(ec);
    }

    /* check if the top/bottom is reached */
    gtk_text_buffer_get_start_iter(ec_sourcebuffer, &start);
    gtk_text_buffer_get_end_iter(ec_sourcebuffer, &end);

    if (!ret) {
        if (ec->backwards ^ inverse) {
            response = utils_yes_no_dialog(
                    _("Top reached, search from bottom?"));
            if (GTK_RESPONSE_YES == response) {
                gtk_text_buffer_place_cursor(ec_sourcebuffer, &end);
                editor_search_next(ec, inverse);
            }
        } else {
            response = utils_yes_no_dialog(
                    _("Bottom reached, search from top?"));
            if (GTK_RESPONSE_YES == response) {
                gtk_text_buffer_place_cursor(ec_sourcebuffer, &start);
                editor_search_next(ec, inverse);
            }
        }
    }
}

void editor_start_replace_next(GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase) {
    L_F_DEBUG;
    GtkTextIter current, mstart, mend;
    gboolean ret = FALSE;

    if (!ec->replace_activated) {
        ec->replace_activated = TRUE;
        editor_start_search(ec, term, backwards, wholeword, matchcase, TRUE);
        return;
    }

    /* place cursor to the next result */
    editor_get_current_iter(ec, &current);

    if (backwards)
       ret = gtk_source_iter_backward_search(&current, term,
                (matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);
    else
       ret = gtk_source_iter_forward_search(&current, term,
                (matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);

    if (ret && (!wholeword || (wholeword
            && gtk_text_iter_starts_word(&mstart)
            && gtk_text_iter_ends_word(&mend)))) {
        gtk_text_buffer_begin_user_action(ec_sourcebuffer);
        gtk_text_buffer_delete(ec_sourcebuffer, &mstart, &mend);
        gtk_text_buffer_insert(ec_sourcebuffer, &mstart, rterm, strlen(rterm));
        gtk_text_buffer_end_user_action(ec_sourcebuffer);
        editor_search_next(ec, FALSE);
    }
    return;
}

void editor_start_replace_all(GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase) {
    L_F_DEBUG;
    GtkTextIter start, mstart, mend;
    gboolean ret = FALSE;

    gtk_text_buffer_get_start_iter(ec_sourcebuffer, &start);

    while (TRUE) {
        ret = gtk_source_iter_forward_search(&start, term,
                (matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);

        if (ret && (!wholeword || (wholeword
                && gtk_text_iter_starts_word(&mstart)
                && gtk_text_iter_ends_word(&mend)))) {
            gtk_text_buffer_begin_user_action(ec_sourcebuffer);
            gtk_text_buffer_delete(ec_sourcebuffer, &mstart, &mend);
            gtk_text_buffer_insert(ec_sourcebuffer, &mstart, rterm,
                    strlen(rterm));
            gtk_text_buffer_end_user_action(ec_sourcebuffer);
            start =  mstart;
        } else break;
    }
}

void editor_get_current_iter(GuEditor* ec, GtkTextIter* current) {
    L_F_DEBUG;
    GtkTextMark* mark;
    mark = gtk_text_buffer_get_insert(ec_sourcebuffer);
    gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, current, mark);
}

void editor_scroll_to_cursor(GuEditor* ec) {
    L_F_DEBUG;
    gtk_text_view_scroll_to_mark(ec_sourceview,
                                 gtk_text_buffer_get_insert(ec_sourcebuffer),
                                 0.25,
                                 FALSE,
                                 0.0,
                                 0.0);
}

void editor_undo_change(GuEditor* ec) {
    L_F_DEBUG;
    GtkTextIter current;
    if (gtk_source_buffer_can_undo(ec->sourcebuffer)) {
        gtk_source_buffer_undo(ec->sourcebuffer);
        editor_get_current_iter(ec, &current);
        editor_scroll_to_cursor(ec);
        gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
    }
}

void editor_redo_change(GuEditor* ec) {
    L_F_DEBUG;
    GtkTextIter current;
    if (gtk_source_buffer_can_redo(ec->sourcebuffer)) {
        gtk_source_buffer_redo(ec->sourcebuffer);
        editor_get_current_iter(ec, &current);
        editor_scroll_to_cursor(ec);
        gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
    }
}
