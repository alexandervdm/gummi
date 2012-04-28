/**
 * @file    editor.c
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


#include "editor.h"

#include <stdlib.h>
#include <string.h>

#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourceiter.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestyleschememanager.h>
#include <gtksourceview/gtksourceview.h>
#ifdef USE_GTKSPELL
#   include <gtkspell/gtkspell.h>
#endif
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "configfile.h"
#include "constants.h"
#include "environment.h"
#include "utils.h"

static void on_inserted_text(GtkTextBuffer *textbuffer,GtkTextIter *location,
                             gchar *text,gint len, gpointer user_data);
static void on_delete_range(GtkTextBuffer *textbuffer,GtkTextIter *start,
                             GtkTextIter *end, gpointer user_data);
                             
const gchar style[][3][20] = {
    { "tool_bold", "\\textbf{", "}" },
    { "tool_italic", "\\emph{", "}" },
    { "tool_unline", "\\underline{", "}" },
    { "tool_left", "\\begin{flushleft}", "\\end{flushleft}"},
    { "tool_center", "\\begin{center}", "\\end{center}"},
    { "tool_right", "\\begin{flushright}", "\\end{flushright}"}
};

GuEditor* editor_new (GuMotion* mc) {
    GuEditor* ec = g_new0 (GuEditor, 1);

    /* File related member initialization */
    ec->workfd = -1;
    ec->fdname = NULL;
    ec->filename = NULL;   /* current opened file name in workspace */
    ec->basename = NULL;   /* use this to form .dvi/.ps/.log etc. files */
    ec->pdffile = NULL;
    ec->workfile = NULL;
    ec->bibfile = NULL;
    ec->projfile = NULL;
    
    GtkSourceLanguageManager* manager = gtk_source_language_manager_new ();
    GtkSourceLanguage* lang = gtk_source_language_manager_get_language (manager,
            "latex");
    ec->buffer = gtk_source_buffer_new_with_language (lang);
    ec->view = GTK_SOURCE_VIEW (gtk_source_view_new_with_buffer (ec->buffer));
    ec->stylemanager = gtk_source_style_scheme_manager_get_default ();
    ec->errortag = gtk_text_tag_new ("error");
    ec->searchtag = gtk_text_tag_new ("search");
    ec->editortags = gtk_text_buffer_get_tag_table (ec_buffer);
    ec->replace_activated = FALSE;
    ec->term = NULL;
    
    gtk_source_view_set_tab_width (ec->view,
            atoi (config_get_value ("tabwidth")));
    gtk_source_view_set_insert_spaces_instead_of_tabs (ec->view,
            TO_BOOL (config_get_value ("spaces_instof_tabs")));
    gtk_source_view_set_auto_indent (ec->view,
            TO_BOOL (config_get_value ("autoindentation")));

#ifdef USE_GTKSPELL
    if (config_get_value ("spelling"))
        editor_activate_spellchecking (ec, TRUE);
#endif

    editor_sourceview_config (ec);
    gtk_text_buffer_set_modified (ec_buffer, FALSE);

    /* Register motion callback */
    ec->sigid[0] = g_signal_connect (ec->view, "key-press-event",
                G_CALLBACK (on_key_press_cb), mc);
    ec->sigid[1] = g_signal_connect (ec->view, "key-release-event",
                G_CALLBACK (on_key_release_cb), mc);
    ec->sigid[2] = g_signal_connect (ec->buffer, "changed",
                G_CALLBACK (check_preview_timer), NULL);

    ec->sigid[3] = g_signal_connect_after(ec->buffer, "insert-text", 
                G_CALLBACK(on_inserted_text), ec);
    ec->sigid[4] = g_signal_connect_after(ec->buffer, "delete-range", 
                G_CALLBACK(on_delete_range), ec);

    return ec;
}

void editor_destroy (GuEditor* ec) {
    gint i = 0;
    
    for (i = 0; i < 2; ++i) {
        if (g_signal_handler_is_connected (ec->view, ec->sigid[i])) {
            g_signal_handler_disconnect (ec->view, ec->sigid[i]);
        }
    }
    for (i = 2; i < 5; ++i) {
        if (g_signal_handler_is_connected (ec->buffer, ec->sigid[i])) {
            g_signal_handler_disconnect (ec->buffer, ec->sigid[i]);
        }
    }

    editor_fileinfo_cleanup (ec);
    g_free(ec);
}

static void on_inserted_text(GtkTextBuffer *textbuffer,GtkTextIter *location,
                             gchar *text,gint len, gpointer user_data) {

    if (location == NULL || user_data == NULL) {
        return;
    }
    GuEditor* e = GU_EDITOR(user_data);
    
    e->last_edit = *location;
    e->sync_to_last_edit = TRUE;
}

static void on_delete_range(GtkTextBuffer *textbuffer,GtkTextIter *start,
                             GtkTextIter *end, gpointer user_data) {

    if (start == NULL || user_data == NULL) {
        return;
    }
    GuEditor* e = GU_EDITOR(user_data);
    
    e->last_edit = *start;
    e->sync_to_last_edit = TRUE;
}

/* FileInfo:
 * When a TeX document includes materials from other files (image, documents,
 * bibliography ... etc), pdflatex will try to find those files under the
 * working directory if the include path is not absolute.
 * Before Gummi svn505, gummi copies the TeX file to a temporarily directory
 * and compile there, because of this, the included files can't be located if
 * the include path is not absolute. In svn505 we copy the TeX file to the
 * same directory as the original TeX document but named it as '.FILENAME.swp'.
 * Since pdflatex refuses to compile TeX files with '.' prefixed, we have to
 * set the environment variable 'openout_any=a'.
 *
 * For a newly created document, all files including the TeX file is stored
 * under the temp directory. For files that are already saved, only the
 * workfile is saved under DIRNAME (FILENAME). Other compilation-related files
 * are located in the temp directory.
 *
 * P.S. pdflatex will automatically strip the suffix, so for a file named
 * FILE.tex under /absolute/path/:
 *
 * filename = /absolute/path/FILE.tex
 * workfile = /absolute/path/.FILE.tex.swp
 * pdffile = /tmp/.FILE.tex.pdf
 */
void editor_fileinfo_update (GuEditor* ec, const gchar* filename) {

    if (ec->workfd != -1)
        editor_fileinfo_cleanup (ec);

    ec->fdname = g_build_filename (C_TMPDIR, "gummi_XXXXXX", NULL);
    ec->workfd = g_mkstemp (ec->fdname);
    
    // This is required for Windows 7, but not for Linux. It may also
    // be the proper way for *nix, but I don't want to change this
    // crucial piece of code at this stage of development -alexander
    #ifdef WIN32
		close(ec->workfd);
	#endif

    if (filename) {
        gchar* base = g_path_get_basename (filename);
        gchar* dir = g_path_get_dirname (filename);
        ec->filename = g_strdup (filename);
        ec->basename = g_strdup_printf ("%s%c.%s", dir, G_DIR_SEPARATOR, base);
        ec->workfile = g_strdup_printf ("%s.swp", ec->basename);
        ec->pdffile =  g_strdup_printf ("%s%c.%s.pdf", C_TMPDIR,
                                       G_DIR_SEPARATOR, base);
        g_free (base);
        g_free (dir);
    } else {
        ec->workfile = g_strdup (ec->fdname);
        ec->basename = g_strdup (ec->fdname);
        ec->pdffile =  g_strdup_printf ("%s.pdf", ec->fdname);
    }
}

gboolean editor_fileinfo_update_biblio (GuEditor* ec,  const gchar* filename) {
    g_free (ec->bibfile);

    if (ec->filename && !g_path_is_absolute (filename)) {
        gchar* dirname = g_path_get_dirname (ec->filename);
        ec->bibfile = g_build_filename (dirname, filename, NULL);
        g_free (dirname);
    } else
        ec->bibfile = g_strdup (filename);
    return utils_path_exists (ec->bibfile);
}

void editor_fileinfo_cleanup (GuEditor* ec) {
    gchar* auxfile = NULL;
    gchar* logfile = NULL;
    gchar* syncfile = NULL;
    
    if (ec->filename) {
        gchar* dirname = g_path_get_dirname (ec->filename);
        gchar* basename = g_path_get_basename (ec->filename);
        auxfile = g_strdup_printf ("%s%c.%s.aux", C_TMPDIR,
                G_DIR_SEPARATOR, basename);
        logfile = g_strdup_printf ("%s%c.%s.log", C_TMPDIR,
                G_DIR_SEPARATOR, basename);
        syncfile = g_strdup_printf ("%s%c.%s.synctex.gz", C_TMPDIR,
                G_DIR_SEPARATOR, basename);
        g_free (basename);
        g_free (dirname);
    } else {
        gchar* dirname = g_path_get_dirname (ec->workfile);
        gchar* basename = g_path_get_basename (ec->workfile);
        auxfile = g_strdup_printf ("%s.aux", ec->fdname);
        logfile = g_strdup_printf ("%s.log", ec->fdname);
        syncfile = g_strdup_printf ("%s.synctex.gz", ec->fdname);
        g_free (basename);
        g_free (dirname);
    }

    // TODO: make a loop or maybe make register of created files? proc? 

    close (ec->workfd);
    ec->workfd = -1;

    g_remove (auxfile);
    g_remove (logfile);
    g_remove (syncfile);
    g_remove (ec->fdname);
    g_remove (ec->workfile);
    g_remove (ec->pdffile);
    g_remove (ec->basename);

    g_free (auxfile);
    g_free (logfile);
    g_free (syncfile);
    g_free (ec->fdname);
    g_free (ec->filename);
    g_free (ec->workfile);
    g_free (ec->pdffile);
    g_free (ec->basename);

    ec->fdname = NULL;
    ec->filename = NULL;
    ec->workfile = NULL;
    ec->pdffile = NULL;
    ec->basename = NULL;
}

void editor_sourceview_config (GuEditor* ec) {
    GtkWrapMode wrapmode = 0;

    gtk_source_buffer_set_highlight_matching_brackets (ec->buffer, TRUE);

    const gchar* style_scheme = config_get_value ("style_scheme");
    editor_set_style_scheme_by_id (ec, style_scheme);

    const gchar* font = config_get_value ("font");
    slog (L_INFO, "setting font to %s\n", font);
    PangoFontDescription* font_desc = pango_font_description_from_string (font);
    gtk_widget_modify_font (GTK_WIDGET (ec->view), font_desc);
    pango_font_description_free (font_desc);

    gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW (ec->view),
            TO_BOOL (config_get_value ("line_numbers")));
    gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW (ec->view),
        TO_BOOL (config_get_value ("highlighting")));

    /* The condition 'textwrapping=False && wordwrapping=True' can't happen */
    if (config_get_value ("textwrapping"))
        wrapmode += 1;
    if (config_get_value ("wordwrapping"))
        wrapmode += 1;

    gtk_text_view_set_wrap_mode (ec_view, wrapmode);
}


#ifdef USE_GTKSPELL
void editor_activate_spellchecking (GuEditor* ec, gboolean status) {
    const gchar* lang = config_get_value ("spell_language");
    GError* err = NULL;
    GError* err2 = NULL;
    GtkSpell* spell = 0;
    if (status) {
        if (! (spell = gtkspell_new_attach (ec_view, "en", &err))) {
            slog (L_ERROR, "gtkspell_new_attach (): %s\n", err->message);
            g_error_free (err);
        }
        if (!gtkspell_set_language (spell, lang, &err2)) {
            slog (L_ERROR, "gtkspell_set_language (): %s\n", err2->message);
            g_error_free (err2);
        }
    } else {
        GtkSpell* spell = gtkspell_get_from_text_view (ec_view);
        if (spell)
            gtkspell_detach (spell);
    }
}
#endif

void editor_fill_buffer (GuEditor* ec, const gchar* text) {
    gtk_text_buffer_begin_user_action (ec_buffer);
    gtk_source_buffer_begin_not_undoable_action (ec->buffer);
    gtk_widget_set_sensitive (GTK_WIDGET (ec->view), FALSE);
    gtk_text_buffer_set_text (ec_buffer, text, strlen (text));
    gtk_widget_set_sensitive (GTK_WIDGET (ec->view), TRUE);
    gtk_source_buffer_end_not_undoable_action (ec->buffer);
    gtk_text_buffer_end_user_action (ec_buffer);

    /* place cursor in the beggining of the document */
    GtkTextIter start;
    gtk_text_buffer_get_start_iter (ec_buffer, &start);
    gtk_text_buffer_place_cursor (ec_buffer, &start);
    gtk_widget_grab_focus (GTK_WIDGET (ec->view));
    ec->sync_to_last_edit = FALSE;
}

gchar* editor_grab_buffer (GuEditor* ec) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (ec_buffer, &start, &end);
    gchar* pstr = gtk_text_iter_get_text (&start, &end);
    return pstr;
}

gboolean editor_buffer_changed (GuEditor* ec) {
    if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (ec->buffer))) {
        return TRUE;
    }
    return FALSE;
}

void editor_insert_package (GuEditor* ec, const gchar* package) {
    GtkTextIter start, mstart, mend, sstart, send;
    gchar* pkgstr = g_strdup_printf ("\\usepackage{%s}\n", package);
    gtk_text_buffer_get_start_iter (ec_buffer, &start);
    gtk_text_iter_forward_search (&start, (gchar*)"\\begin{document}", 0,
            &mstart, &mend, NULL);
    if (!gtk_text_iter_backward_search (&mstart, pkgstr, 0, &sstart, &send,
                NULL)) {
        gtk_source_buffer_begin_not_undoable_action (ec->buffer);
        gtk_text_buffer_begin_user_action (ec_buffer);
        gtk_text_buffer_insert (ec_buffer, &mstart, pkgstr, -1);
        gtk_text_buffer_end_user_action (ec_buffer);
        gtk_source_buffer_end_not_undoable_action (ec->buffer);
        gtk_text_buffer_set_modified (ec_buffer, TRUE);
    }
    g_free (pkgstr);
}

void editor_insert_bib (GuEditor* ec, const gchar* package) {
    GtkTextIter start, end, mstart, mend, sstart, send;
    gchar* pkgstr = g_strdup_printf (
            "\\bibliography{%s}{}\n\\bibliographystyle{plain}\n", package);
    gtk_text_buffer_get_start_iter (ec_buffer, &start);
    gtk_text_buffer_get_end_iter (ec_buffer, &end);
    gtk_text_iter_backward_search (&end, (gchar*)"\\end{document}", 0,
            &mstart, &mend, NULL);
    if (!gtk_text_iter_forward_search (&start, "\\bibliography{", 0,
                &sstart, &send, NULL)) {
        gtk_source_buffer_begin_not_undoable_action (ec->buffer);
        gtk_text_buffer_begin_user_action (ec_buffer);
        gtk_text_buffer_insert (ec_buffer, &mstart, pkgstr, -1);
        gtk_text_buffer_end_user_action (ec_buffer);
        gtk_source_buffer_end_not_undoable_action (ec->buffer);
        gtk_text_buffer_set_modified (ec_buffer, TRUE);
    }
    g_free (pkgstr);
}

void editor_set_selection_textstyle (GuEditor* ec, const gchar* type) {
    GtkTextIter start, end;
    gint i = 0, selected = 0;
    const gchar* selected_text = 0;
    gint style_size = sizeof (style) / sizeof (style[0]);
    gchar** result = NULL;
    GError* err = NULL;
    GRegex* match_str = NULL;
    GMatchInfo* match_info = NULL;
    gchar* outtext = NULL;
    gchar* regexbuf = NULL;

    /* grab selected text */
    gtk_text_buffer_get_selection_bounds (ec_buffer, &start, &end);
    selected_text = gtk_text_iter_get_text (&start, &end);

    /* select style */
    for (i = 0; i < style_size; ++i)
        if (STR_EQU (style[i][0], type)) {
            selected = i;
            break;
        }

    /* generate regex expression */
    regexbuf = g_strdup_printf ("(.*)%s%s(.*)%s%s(.*)",
            (style[selected][1][0] == '\\')? "\\": "",
            style[selected][1],
            (style[selected][2][0] == '\\')? "\\": "",
            style[selected][2]);

    if (! (match_str = g_regex_new (regexbuf, G_REGEX_DOTALL, 0, &err))) {
        slog (L_ERROR, "g_regex_new (): %s\n", err->message);
        g_error_free (err);
        goto cleanup;
    }

    if (g_regex_match (match_str, selected_text, 0, &match_info)) {
        result = g_match_info_fetch_all (match_info);
        if (strlen (result[1]) == 0 && strlen (result[3]) == 0) {
            /* already applied, so we remove it */
            outtext = g_strdup (result[2]);
        } else if (strlen (result[1]) != 0 || strlen (result[3]) != 0) {
            /* the text contains a partially styled text, remove it and apply
             * style to the whole text */
            outtext = g_strdup_printf ("%s%s%s%s%s",
                    style[selected][1], result[1], result[2], result[3],
                    style[selected][2]);
        }
    } else { /* no previous style applied */
        outtext = g_strdup_printf ("%s%s%s",
                style[selected][1], selected_text, style[selected][2]);
    }

    gtk_text_buffer_begin_user_action (ec_buffer);
    gtk_text_buffer_delete (ec_buffer, &start, &end);
    gtk_text_buffer_insert (ec_buffer, &start, outtext, -1);
    end = start;
    gtk_text_iter_backward_chars (&start, strlen (outtext));
    gtk_text_buffer_select_range (ec_buffer, &start, &end);
    gtk_text_buffer_end_user_action (ec_buffer);
    gtk_text_buffer_set_modified (ec_buffer, TRUE);

cleanup:
    g_free (outtext);
    g_free (regexbuf);
    g_strfreev (result);
    g_match_info_free (match_info);
    g_regex_unref (match_str);
}

void editor_apply_errortags (GuEditor* ec, gint* lines) {
    GtkTextIter start, end;
    gint count = 0;
    /* remove the tag from the table if it is in threre */
    if (gtk_text_tag_table_lookup (ec->editortags, "error"))
        gtk_text_tag_table_remove (ec->editortags, ec->errortag);

    gtk_text_tag_table_add (ec->editortags, ec->errortag);
    while (lines[count]) {
        gtk_text_buffer_get_iter_at_line (ec_buffer,&start,lines[count]-1);
        gtk_text_buffer_get_iter_at_line (ec_buffer, &end, lines[count]);
        gtk_text_buffer_apply_tag (ec_buffer, ec->errortag, &start, &end);
        ++count;
    }
}

void editor_jumpto_search_result (GuEditor* ec, gint direction) {
    if (!ec->term) return;
    if (direction == 1)
        editor_search_next (ec, FALSE);
    else
        editor_search_next (ec, TRUE);
}

void editor_start_search (GuEditor* ec, const gchar* term,
        gboolean backwards, gboolean wholeword, gboolean matchcase) {
    /* save options */
    if (ec->term != term) {
        g_free (ec->term);
        ec->term = g_strdup (term);
    }

    ec->backwards = backwards;
    ec->wholeword = wholeword;
    ec->matchcase = matchcase;

    editor_apply_searchtag (ec);
    editor_search_next (ec, FALSE);
}

void editor_apply_searchtag (GuEditor* ec) {
    GtkTextIter start, mstart, mend;
    gboolean ret = FALSE;

    gtk_text_buffer_get_start_iter (ec_buffer, &start);

    if (gtk_text_tag_table_lookup (ec->editortags, "search"))
        gtk_text_tag_table_remove (ec->editortags, ec->searchtag);
    gtk_text_tag_table_add (ec->editortags, ec->searchtag);

    while (TRUE) {
        ret = gtk_source_iter_forward_search (&start, ec->term,
                (ec->matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);

        if (ret && (!ec->wholeword || (ec->wholeword
                        && gtk_text_iter_starts_word (&mstart)
                        && gtk_text_iter_ends_word (&mend)))) {
            gtk_text_buffer_apply_tag (ec_buffer, ec->searchtag,
                    &mstart, &mend);
            start =  mend;
        } else break;
    }
}

void editor_search_next (GuEditor* ec, gboolean inverse) {
    GtkTextIter current, start, end, mstart, mend;
    gboolean ret = FALSE, response = FALSE;

    editor_get_current_iter (ec, &current);

    if (ec->backwards ^ inverse) {
        ret = gtk_source_iter_backward_search (&current, ec->term,
                (ec->matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);
    } else {
        gtk_text_iter_forward_chars (&current, strlen (ec->term));
        ret = gtk_source_iter_forward_search (&current, ec->term,
                (ec->matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);
    }

    if (ret && (!ec->wholeword || (ec->wholeword
            && gtk_text_iter_starts_word (&mstart)
            && gtk_text_iter_ends_word (&mend)))) {
        gtk_text_buffer_select_range (ec_buffer, &mstart, &mend);
        editor_scroll_to_cursor (ec);
    }
    /* check if the top/bottom is reached */
    gtk_text_buffer_get_start_iter (ec_buffer, &start);
    gtk_text_buffer_get_end_iter (ec_buffer, &end);

    if (!ret) {
        if (ec->backwards ^ inverse) {
            response = utils_yes_no_dialog (
                    _("Top reached, search from bottom?"));
            if (GTK_RESPONSE_YES == response) {
                gtk_text_buffer_place_cursor (ec_buffer, &end);
                editor_search_next (ec, inverse);
            }
        } else {
            response = utils_yes_no_dialog (
                    _("Bottom reached, search from top?"));
            if (GTK_RESPONSE_YES == response) {
                gtk_text_buffer_place_cursor (ec_buffer, &start);
                editor_search_next (ec, inverse);
            }
        }
    }
}

void editor_start_replace_next (GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase) {
    GtkTextIter current, mstart, mend;
    gboolean ret = FALSE;

    if (!ec->replace_activated) {
        ec->replace_activated = TRUE;
        editor_start_search (ec, term, backwards, wholeword, matchcase);
        return;
    }

    /* place cursor to the next result */
    editor_get_current_iter (ec, &current);

    if (backwards)
       ret = gtk_source_iter_backward_search (&current, term,
                (matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);
    else
       ret = gtk_source_iter_forward_search (&current, term,
                (matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);

    if (ret && (!wholeword || (wholeword
            && gtk_text_iter_starts_word (&mstart)
            && gtk_text_iter_ends_word (&mend)))) {
        gtk_text_buffer_begin_user_action (ec_buffer);
        gtk_text_buffer_delete (ec_buffer, &mstart, &mend);
        gtk_text_buffer_insert (ec_buffer, &mstart, rterm, -1);
        gtk_text_buffer_end_user_action (ec_buffer);
        editor_search_next (ec, FALSE);
    }
    return;
}

void editor_start_replace_all (GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase) {
    GtkTextIter start, mstart, mend;
    gboolean ret = FALSE;

    gtk_text_buffer_get_start_iter (ec_buffer, &start);

    while (TRUE) {
        ret = gtk_source_iter_forward_search (&start, term,
                (matchcase? 0: GTK_SOURCE_SEARCH_CASE_INSENSITIVE),
                &mstart, &mend, NULL);

        if (ret && (!wholeword || (wholeword
                && gtk_text_iter_starts_word (&mstart)
                && gtk_text_iter_ends_word (&mend)))) {
            gtk_text_buffer_begin_user_action (ec_buffer);
            gtk_text_buffer_delete (ec_buffer, &mstart, &mend);
            gtk_text_buffer_insert (ec_buffer, &mstart, rterm, -1);
            gtk_text_buffer_end_user_action (ec_buffer);
            start =  mstart;
        } else break;
    }
}

void editor_get_current_iter (GuEditor* ec, GtkTextIter* current) {
    GtkTextMark* mark = gtk_text_buffer_get_insert (ec_buffer);
    gtk_text_buffer_get_iter_at_mark (ec_buffer, current, mark);
}

void editor_scroll_to_cursor (GuEditor* ec) {
    gtk_text_view_scroll_to_mark (ec_view,
                                 gtk_text_buffer_get_insert (ec_buffer),
                                 0.25, FALSE, 0.0, 0.0);
}

void editor_scroll_to_line (GuEditor* ec, gint line) {
    if (ec == NULL) {
        return;
    }
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(ec_buffer, &iter, line);
    gtk_text_buffer_place_cursor(ec_buffer, &iter);
    editor_scroll_to_cursor(ec);
    ec->sync_to_last_edit = FALSE;
}

void editor_undo_change (GuEditor* ec) {
    GtkTextIter current;
    if (gtk_source_buffer_can_undo (ec->buffer)) {
        gtk_source_buffer_undo (ec->buffer);
        editor_get_current_iter (ec, &current);
        editor_scroll_to_cursor (ec);
        gtk_text_buffer_set_modified (ec_buffer, TRUE);
    }
}

void editor_redo_change (GuEditor* ec) {
    GtkTextIter current;
    if (gtk_source_buffer_can_redo (ec->buffer)) {
        gtk_source_buffer_redo (ec->buffer);
        editor_get_current_iter (ec, &current);
        editor_scroll_to_cursor (ec);
        gtk_text_buffer_set_modified (ec_buffer, TRUE);
    }
}

void editor_set_style_scheme_by_id (GuEditor* ec, const gchar* id) {
    
    GtkSourceStyleScheme* scheme =
        gtk_source_style_scheme_manager_get_scheme (ec->stylemanager, id);
    slog (L_INFO, "setting styles scheme to %s\n", id);

    if (!scheme) {
        slog (L_ERROR, "no style scheme %s found, setting to classic\n", id);
        scheme = gtk_source_style_scheme_manager_get_scheme (ec->stylemanager,
                "classic");
    }
    gtk_source_buffer_set_style_scheme (ec->buffer, scheme);
    
    set_style_fg_bg(G_OBJECT (ec->searchtag), scheme, "search-match", "yellow");
    set_style_fg_bg(G_OBJECT (ec->errortag), scheme,"def:error",  "red");
}


static inline gdouble gdkcolor_luminance(GdkColor c) {
    return (0.2126 * c.red + 0.7152 * c.green + 0.0722 * c.blue) / G_MAXUINT16;
}


/**
 *  Sets a object's fore- and background color to that of scheme's style 
 *  "styleName". If no background color is defined in the style, defaultBG is 
 *  used. defaultBG can be any valid parameter to gdk_color_parse().
 *  If only a foreground color was defined and it has not enough contrast to the
 *  default background, it will be overwritten. The foreground color will either
 *  be white or black, which has more contrast.
 */
void set_style_fg_bg (GObject* obj, GtkSourceStyleScheme* scheme, 
                      gchar* styleName, gchar* defaultBG) {
    GtkSourceStyle *style = NULL;
    
    gchar *bg = NULL;
    gchar *fg = NULL;
    gboolean foreground_set;
    gboolean background_set;
    GdkColor foreground;
    GdkColor background;


    if (scheme == NULL) {
        goto set_style_fg_bg_return_defaults;
    }
    
    style = gtk_source_style_scheme_get_style (scheme, styleName);
    
    if (style == NULL) {
        goto set_style_fg_bg_return_defaults;
    }
    
    // Get properties of style
    g_object_get (style, 
                  "foreground-set", &foreground_set, 
                  "foreground", &fg,
                  "background-set", &background_set, 
                  "background", &bg,
                  NULL);

    if (foreground_set) {
        if (fg == NULL || !gdk_color_parse (fg, &foreground))
            foreground_set = FALSE;
    }

    if (background_set) {
        if (bg == NULL || !gdk_color_parse (bg, &background))
            background_set = FALSE;
    }    

    g_free(fg);
    g_free(bg);
    
    if (background_set && foreground_set) {
        // We trust the style to set both to good values
        // Do nothing
    } else if (!background_set && foreground_set) {
        // Set bg to default and check if fg has enough contrast
        gdk_color_parse(defaultBG, &background);
        gdouble diff = ABS(gdkcolor_luminance(foreground) -
                gdkcolor_luminance(background));
        if (diff < 0.5) {
            slog(L_INFO, "Style \"%s\" defines a foreground, but no background "
                         "color. As the fourground color has not enough "
                         "contrast to Gummis default background color, the "
                         "foreground color has been adjusted.\n", styleName);
            if (gdkcolor_luminance(background) > 0.5) {
                gdk_color_parse("black", &foreground);
            } else {
                gdk_color_parse("white", &foreground);
            }
        }
    } else if (background_set && !foreground_set) {
        // Choose a fg = white or black, which has more contrast   
        if (gdkcolor_luminance(background) > 0.5) {
            gdk_color_parse("black", &foreground);
        } else {
            gdk_color_parse("white", &foreground);
        }
    } else {
        // none set, set defaults
        goto set_style_fg_bg_return_defaults;
    }
    
    g_object_set (obj, "foreground-gdk", &foreground, 
                        "background-gdk", &background, NULL);
    return;
                        
set_style_fg_bg_return_defaults:

    // No valid style, set defaults
    gdk_color_parse(defaultBG, &background);
    if (gdkcolor_luminance(background) > 0.5) {
        gdk_color_parse("black", &foreground);
    } else {
        gdk_color_parse("white", &foreground);
    }
    
    g_object_set (obj, "foreground-gdk", &foreground, 
                        "background-gdk", &background, NULL);
                        
}

/* The following functions are taken from gedit and partially modified */

gint schemes_compare (gconstpointer a, gconstpointer b) {
    GtkSourceStyleScheme *scheme_a = (GtkSourceStyleScheme *)a;
    GtkSourceStyleScheme *scheme_b = (GtkSourceStyleScheme *)b;

    const gchar *name_a = gtk_source_style_scheme_get_name (scheme_a);
    const gchar *name_b = gtk_source_style_scheme_get_name (scheme_b);

    return g_utf8_collate (name_a, name_b);
}

GList* editor_list_style_scheme_sorted (void) {
    const gchar * const * scheme_ids;
    GList *schemes = NULL;
    GtkSourceStyleSchemeManager* manager =
        gtk_source_style_scheme_manager_get_default();

    scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);

    while (*scheme_ids != NULL) {
        GtkSourceStyleScheme *scheme;
        scheme = gtk_source_style_scheme_manager_get_scheme (manager,
                *scheme_ids);
        schemes = g_list_prepend (schemes, scheme);
        ++scheme_ids;
    }

    if (schemes != NULL)
        schemes = g_list_sort (schemes, (GCompareFunc)schemes_compare);

    return schemes;
}
