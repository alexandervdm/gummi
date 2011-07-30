/**
 * @file   latex.c
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

#include "latex.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "configfile.h"
#include "editor.h"
#include "environment.h"
#include "latex.h"
#include "utils.h"
#include "gui/gui-preview.h"


#ifdef WIN32
    const gchar *cmdsep = "&&";
    const gchar *cmdpms = "";
#else
    const gchar *cmdpms = "env openout_any=a";
    const gchar *cmdsep = ";";
#endif


GuLatex* latex_init (void) {
    GuLatex* l = g_new0 (GuLatex, 1);
    l->errormessage = NULL;
    l->modified_since_compile = FALSE;
    return l;
}

gchar* latex_update_workfile (GuLatex* lc, GuEditor* ec) {
    GtkTextIter current, start, end;
    gchar *text;
    
    /* save selection */
    editor_get_current_iter (ec, &current);
    gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (ec->buffer), &start,
            &end);
    text = editor_grab_buffer (ec);

    /* restore selection */
    gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (ec->buffer), &current);
    gtk_text_buffer_select_range (GTK_TEXT_BUFFER (ec->buffer), &end, &start);
    
    /* write buffer content to the workfile */
    utils_set_file_contents (ec->workfile, text, -1);
    
    return text;
}

void latex_update_pdffile (GuLatex* lc, GuEditor* ec) {
    if (!lc->modified_since_compile) return;

    const gchar* typesetter = config_get_value ("typesetter");
    if (!g_find_program_in_path (typesetter)) {
        slog (L_G_ERROR, "Typesetter command `%s' not found, setting to "
                "pdflatex.\n", typesetter);
        config_set_value ("typesetter", "pdflatex");
    }
    gchar* dirname = g_path_get_dirname (ec->workfile);
    gchar* command = g_strdup_printf ("cd \"%s\"%s "
                                     "%s %s "
                                     "-interaction=nonstopmode "
                                     "-file-line-error "
                                     "-halt-on-error"
                                     "%s "
                                     "-output-directory=\"%s\" \"%s\"",
                                     dirname,
                                     cmdsep,
                                     cmdpms,
                                     config_get_value ("typesetter"),
                                     config_get_value ("extra_flags"),
                                     ec->tmpdir,
                                     ec->workfile);
    g_free (dirname);

    previewgui_update_statuslight ("gtk-refresh");
 
    g_free (lc->errormessage);

    Tuple2 cresult = utils_popen_r (command);
    memset (lc->errorlines, 0, BUFSIZ);
    lc->errormessage = (gchar*)cresult.second;
    lc->modified_since_compile = FALSE;

    /* find error line */
    if ((gint)cresult.first) {
        gchar* result = NULL;
        GError* err = NULL;
        GRegex* match_str = NULL;
        GMatchInfo* match_info;

        if (! (match_str = g_regex_new (":(\\d+):", G_REGEX_DOTALL, 0, &err))) {
            slog (L_ERROR, "g_regex_new (): %s\n", err->message);
            g_error_free (err);
            return;
        }

        if (g_regex_match (match_str, (gchar*)cresult.second, 0, &match_info)) {
            gint count = 0;
            while (g_match_info_matches (match_info)) {
                if (count + 1 == BUFSIZ) break;
                result = g_match_info_fetch (match_info, 1);
                lc->errorlines[count++] = atoi (result);
                g_free (result);
                g_match_info_next (match_info, NULL);
            }
        }
        if (!lc->errorlines[0])
            lc->errorlines[0] = -1;
        g_match_info_free (match_info);
        g_regex_unref (match_str);

        previewgui_update_statuslight ("gtk-no");
    } else
        previewgui_update_statuslight ("gtk-yes");
    g_free (command);
}

void latex_update_auxfile (GuLatex* lc, GuEditor* ec) {
    gchar* dirname = g_path_get_dirname (ec->workfile);
    gchar* command = g_strdup_printf ("cd \"%s\"%s"
                                     "%s %s "
                                     "--draftmode "
                                     "-interaction=nonstopmode "
                                     "--output-directory=\"%s\" \"%s\"",
                                     dirname,
                                     cmdsep,
                                     cmdpms,
                                     config_get_value ("typesetter"),
                                     ec->tmpdir,
                                     ec->workfile);
    g_free (dirname);
    Tuple2 res = utils_popen_r (command);
    g_free (res.second);
    g_free (command);
}

gboolean latex_precompile_check (gchar* editortext) {
    /* both documentclass and documentstyle appear to be valid.
     * http://pangea.stanford.edu/computing/unix/formatting/parts.php
     * TOD: Improve and add document scan tags and make compatible with
     * upcoming master/slave document system */
    
    gboolean class = utils_subinstr("\\documentclass", editortext, FALSE);
    gboolean style = utils_subinstr("\\documentstyle", editortext, FALSE);
    
    return (class || style);
}

void latex_export_pdffile (GuLatex* lc, GuEditor* ec, const gchar* path,
        gboolean prompt_overrite) {
    gchar* savepath = NULL;
    GError* err = NULL;
    gint ret = 0;

    if (0 != strcmp (path + strlen (path) -4, ".pdf"))
        savepath = g_strdup_printf ("%s.pdf", path);
    else
        savepath = g_strdup (path);

    if (prompt_overrite && utils_path_exists (savepath)) {
        ret = utils_yes_no_dialog (_("The file already exists. Overwrite?"));
        if (GTK_RESPONSE_YES != ret) {
            g_free (savepath);
            return;
        }
    }
    if (!utils_copy_file (ec->pdffile, savepath, &err)) {
        slog (L_G_ERROR, "utils_copy_file (): %s\n", err->message);
        g_error_free (err);
    }

    g_free (savepath);
}
