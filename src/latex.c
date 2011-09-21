/**
 * @file   latex.c
 * @brief  
 *
 * Copyright (C) 2010-2011 Gummi-Dev Team <alexvandermey@gmail.com>
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
#include "constants.h"
#include "editor.h"
#include "environment.h"
#include "gui/gui-preview.h"
#include "utils.h"

#include "compile/rubber.h"
#include "compile/latexmk.h"
#include "compile/texlive.h"



GuLatex* latex_init (void) {
    GuLatex* l = g_new0 (GuLatex, 1);
    l->errormessage = NULL;
    l->modified_since_compile = FALSE;
    
    texlive_init ();
    rubber_init ();
    
    return l;
}



gboolean latex_method_active (gchar* method) {
    if (g_strcmp0 (config_get_value("compile_method"), method) == 0) {
        return TRUE;
    }
    return FALSE;
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




gchar* latex_set_compile_cmd (GuEditor* ec) {
    
    const gchar *typesetter = config_get_value ("typesetter");
    gchar* dirname = g_path_get_dirname (ec->workfile);
    gchar *setup = g_strdup_printf("cd \"%s\"%s %s", dirname, C_CMDSEP, C_TEXSEC);
    gchar *flags = NULL;
    gchar *outdir = NULL;
    
    if (rubber_active()) {
        

            
        
        if (latex_method_active("texpdf")) flags = g_strdup_printf("-d -q");
        if (latex_method_active("texdvipspdf")) flags = g_strdup_printf("-p -d -q");
        outdir = g_strdup_printf("--into=\"%s\"", C_TMPDIR);
    }
    else { /* pdflatex/xelatex */
        flags = g_strdup_printf("-interaction=nonstopmode "
                                     "-file-line-error "
                                     "-halt-on-error");
        outdir = g_strdup_printf("-output-directory=\"%s\"", C_TMPDIR);
    }
    
    gchar* command = g_strdup_printf ("%s %s %s %s \"%s\"", 
                                        setup, 
                                        typesetter, 
                                        flags, 
                                        outdir, 
                                        ec->workfile);
    
    g_free (dirname);
    g_free (flags);
    g_free (outdir);
    return command;
}

gchar* latex_analyse_log (gchar *compile_log) {
    
    /* Rubber does not post the pdftex compilation output to tty, so we will
     * have to open the log file and retrieve it I guess */
    if (rubber_active()) {
        gchar* logname = NULL;
        /* TODO: integrate - retrieve from functions */
        gchar *filename = g_strdup (gummi_get_active_editor()->filename);
        gchar *workfile = g_strdup (gummi_get_active_editor()->workfile);
        gchar *pdffile = g_strdup (gummi_get_active_editor()->pdffile);

        if (filename == NULL) {
            logname = g_strconcat (workfile, ".log", NULL);
            }
        else {
            logname = g_strdup (pdffile);
            logname[strlen (logname) -4] = 0;
            logname = g_strconcat (logname, ".log", NULL);
            }
            
        g_file_get_contents (logname, &compile_log, NULL, NULL);
        g_free (filename);
        g_free (workfile);
        g_free (pdffile);
    }
    return compile_log;
}

            
    

void latex_analyse_errors (GuLatex* lc) {
    gchar* result = NULL;
    GError* err = NULL;
    GRegex* match_str = NULL;
    GMatchInfo* match_info;

    if (! (match_str = g_regex_new (":(\\d+):", G_REGEX_DOTALL, 0, &err))) {
        slog (L_ERROR, "g_regex_new (): %s\n", err->message);
        g_error_free (err);
        return;
    }

    if (g_regex_match (match_str, lc->errormessage, 0, &match_info)) {
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
    
}

void latex_update_pdffile (GuLatex* lc, GuEditor* ec) {
    if (!lc->modified_since_compile) return;

    const gchar* typesetter = config_get_value ("typesetter");
    if (!utils_program_exists (typesetter)) {
        /* L_G_ERROR inside the thread freezes up 
        slog (L_G_ERROR, "Typesetter command \"%s\" not found, setting to "
                "pdflatex.\n", typesetter);*/
                
        /* Set to default first detected typesetter */
        config_set_value ("typesetter", "pdflatex");
    }

    /* create compile command */
    gchar *command = latex_set_compile_cmd (ec);

    g_free (lc->errormessage);
    memset (lc->errorlines, 0, BUFSIZ);
    
    /* run pdf compilation */
    Tuple2 cresult = utils_popen_r (command);
    gboolean cerrors = (gint)cresult.first;
    gchar* coutput = (gchar*)cresult.second;
    
    lc->errormessage = latex_analyse_log (coutput);
    lc->modified_since_compile = FALSE;
    
    /* find error line */
    if (cerrors) {
        latex_analyse_errors (lc);
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
                                     C_CMDSEP,
                                     C_TEXSEC,
                                     config_get_value ("typesetter"),
                                     C_TMPDIR,
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
