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
#include "external.h"
#include "gui/gui-preview.h"
#include "utils.h"

#include "compile/rubber.h"
#include "compile/latexmk.h"
#include "compile/texlive.h"

extern Gummi* gummi;

GuLatex* latex_init (void) {
    GuLatex* l = g_new0 (GuLatex, 1);
    l->compilelog = NULL;
    l->modified_since_compile = FALSE;
    
    l->tex_version = texlive_init ();
    rubber_init ();
    latexmk_init ();
    
    /* TODO: Temp hard set of compilation options for migrating configs */
    if (strlen(config_get_value("typesetter")) == 0)
        config_set_value("typesetter", "pdflatex");
    if (strlen(config_get_value("compile_steps")) == 0)
        config_set_value("compile_steps", "texpdf");
    
    return l;
}



gboolean latex_method_active (gchar* method) {
    if (utils_strequal (config_get_value ("compile_steps"), method)) {
        return TRUE;
    }
    return FALSE;
}

gchar* latex_update_workfile (GuLatex* lc, GuEditor* ec) {
    gchar *text;
    
    text = editor_grab_buffer (ec);
    
    /* write buffer content to the workfile */
    utils_set_file_contents (ec->workfile, text, -1);
    
    return text;
}




gchar* latex_set_compile_cmd (GuEditor* ec) {
    
    const gchar* method = config_get_value ("compile_steps");
    const gchar* curdir = g_path_get_dirname (ec->workfile);
    const gchar* precommand = g_strdup_printf ("cd \"%s\"%s%s", 
                                                curdir, C_CMDSEP, C_TEXSEC);
                                                
    gchar* texcmd = NULL;
    
    if (rubber_active()) {
        texcmd = rubber_get_command (method, ec->workfile);
    }
    else if (latexmk_active()) {
        texcmd = latexmk_get_command (method, ec->workfile, ec->basename);
    }
    else {
        texcmd = texlive_get_command (method, ec->workfile, ec->basename);
    }

    gchar* combined = g_strdup_printf ("%s %s", 
                                        precommand, 
                                        texcmd);
                                        
    //printf("%s\n", combined);

    return combined;
}

gchar* latex_analyse_log (gchar* log, gchar* filename, gchar* basename) {
    /* Rubber does not post the pdftex compilation output to tty, so we will
     * have to open the log file and retrieve it I guess */
    if (rubber_active()) {
        gchar* logpath = NULL;
        if (filename == NULL) {
            logpath = g_strconcat (basename, ".log", NULL);
        }
        else {
            logpath = g_strconcat (C_TMPDIR, C_DIRSEP, 
                                   g_path_get_basename(basename), ".log", NULL);
        }
        g_file_get_contents (logpath, &log, NULL, NULL);
    }
    return log;
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
    
    if (lc->compilelog == NULL) printf("null\n");

    if (g_regex_match (match_str, lc->compilelog, 0, &match_info)) {
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

gboolean latex_update_pdffile (GuLatex* lc, GuEditor* ec) {
    static glong cerrors = 0;
    gchar* basename = ec->basename;
    gchar* filename = ec->filename;

    if (!lc->modified_since_compile) return cerrors == 0;

    const gchar* typesetter = config_get_value ("typesetter");
    if (!external_exists (typesetter)) {
        /* Set to default first detected typesetter */
        config_set_value ("typesetter", "pdflatex");
    }

    /* create compile command */
    gchar *command = latex_set_compile_cmd (ec);

    g_free (lc->compilelog);
    memset (lc->errorlines, 0, BUFSIZ);
    
    /* run pdf compilation */
    Tuple2 cresult = utils_popen_r (command);
    cerrors = (glong)cresult.first;
    gchar* coutput = (gchar*)cresult.second;
    
    lc->compilelog = latex_analyse_log (coutput, filename, basename);
    lc->modified_since_compile = FALSE;
    
    /* find error line */
    if (cerrors && (g_utf8_strlen (lc->compilelog, -1) != 0)) {
        latex_analyse_errors (lc);
    }
    
    g_free (command);

    return cerrors == 0;
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

    if (0 != g_strcmp0 (path + strlen (path) -4, ".pdf"))
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
        slog (L_G_ERROR, 
                    _("Unable to export PDF..\nDocument contains errors.\n"));
        g_error_free (err);
    }

    g_free (savepath);
}

gboolean latex_run_makeindex (GuEditor* ec) {
    int retcode;
    
    if (g_find_program_in_path ("makeindex")) {
        
        const gchar* precmd = g_strdup_printf ("cd \"%s\"%s%s", 
                                                C_TMPDIR, C_CMDSEP, C_TEXSEC);
                                                    
        gchar* command = g_strdup_printf ("%s makeindex \"%s.idx\"",
                                         precmd, 
                                         g_path_get_basename(ec->basename));
                                         
        Tuple2 res = utils_popen_r (command);
        retcode = (glong)res.first;
        g_free (command);
    }
    if (retcode == 0) return TRUE;
    return FALSE;
}

gboolean latex_can_synctex (void) {
    if (gummi->latex->tex_version >= 2008) {
        if (!rubber_active()) {
            return TRUE;
        }
    }
    return FALSE;
}


gboolean latex_use_synctex (void) {
    return (config_get_value("synctex") && config_get_value("autosync"));
}

gboolean latex_use_shellescaping (void) {
    if (config_get_value ("shellescape")) return TRUE;
    return FALSE;
}



