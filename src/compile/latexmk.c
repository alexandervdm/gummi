/**
 * @file   latexmk.c
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

#include "latexmk.h"

#include "configfile.h"
#include "constants.h"
#include "external.h"
#include "utils.h"

gboolean lmk_detected = FALSE;

void latexmk_init (void) {
    
    if (external_exists (C_LATEXMK)) {
        // TODO: check if supported version
        slog (L_INFO, "Typesetter detected: Latexmk %s\n", 
                       external_version (C_LATEXMK));
        lmk_detected = TRUE;
    }
}

gboolean latexmk_active (void) {
    if (g_str_equal (config_get_value("typesetter"), C_LATEXMK)) {
        return TRUE;
    }
    return FALSE;
}

gboolean latexmk_detected (void) {
    return lmk_detected;
}

gchar* latexmk_get_command (const gchar* method, gchar* workfile, gchar* basename) {
    gchar* outdir = g_strdup("");
    gchar* base;

    /* reroute output files to our temp directory */
    if (!g_str_equal (C_TMPDIR, g_path_get_dirname (workfile))) {
        base = g_path_get_basename (basename);
        outdir = g_strdup_printf ("-jobname=\"%s/%s\"", C_TMPDIR, base);
    }
    
    const gchar* flags = latexmk_get_flags (method);
    gchar* lmkcmd;
    
    lmkcmd = g_strdup_printf("latexmk %s %s \"%s\"", flags, outdir, workfile);
    return lmkcmd;
}


gchar* latexmk_get_flags (const gchar *method) {
    gchar* lmkwithoutput;
    gchar* lmkflags;

    if (config_get_value("synctex")) {
        if (g_str_equal (method, "texpdf")) {
            lmkflags = g_strdup_printf("-e \"\\$pdflatex = 'pdflatex -synctex=1'\" -silent");
        }
        else {
            lmkflags = g_strdup_printf("-e \"\\$latex = 'latex -synctex=1'\" -silent");
        }
    }
    else {
        if (g_str_equal (method, "texpdf")) {
            lmkflags = g_strdup_printf("-e \"\\$pdflatex = 'pdflatex -synctex=0'\" -silent");
        }
        else {
            lmkflags = g_strdup_printf("-e \"\\$latex = 'latex -synctex=0'\" -silent");
        }
    }
    
    if (g_str_equal (method, "texpdf")) {
        lmkwithoutput = g_strconcat (lmkflags, " -pdf", NULL);
    }
    else if (g_str_equal (method, "texdvipdf")) {
        lmkwithoutput = g_strconcat (lmkflags, " -pdfdvi", NULL);
    }
    else {
        lmkwithoutput = g_strconcat (lmkflags, " -pdfps", NULL);
    }
    
    g_free (lmkflags);
    return lmkwithoutput;
}




