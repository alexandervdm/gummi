/**
 * @file   texlive.c
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

#include "texlive.h"

#include <glib.h>

#include "configfile.h"
#include "constants.h"
#include "utils.h"

gboolean pdf_detected = FALSE;
gboolean xel_detected = FALSE;


/* All the functions for "pure" building with texlive only tools */

void texlive_init (void) {
    // TODO: check if supported version
    if (utils_program_exists (C_PDFLATEX)) {
        slog (L_INFO, "Typesetter detected: %s\n",
              utils_get_version (C_PDFLATEX));
        pdf_detected = TRUE;
    }
    if (utils_program_exists (C_XELATEX)) {
        slog (L_INFO, "Typesetter detected: %s\n",
              utils_get_version (C_XELATEX));
        xel_detected = TRUE;
    }
    
}

gboolean texlive_active (void) {
    if (pdflatex_active() || xelatex_active()) {
        return TRUE;
    }
    return FALSE;
}

gboolean pdflatex_active (void) {
    if (utils_strequal (config_get_value("typesetter"), "pdflatex")) {
        return TRUE;
    }
    return FALSE;
}

gboolean xelatex_active (void) {
    if (utils_strequal (config_get_value("typesetter"), "xelatex")) {
        return TRUE;
    }
    return FALSE;
}

gboolean pdflatex_detected (void) {
    return pdf_detected;
}

gboolean xelatex_detected (void) {
    return xel_detected;
}

gchar* texlive_get_command (const gchar* method, gchar* workfile, gchar* basename) {
    
    const gchar* outdir = g_strdup_printf("-output-directory=\"%s\"", C_TMPDIR);
    
    
    gchar *typesetter = NULL;
    gchar *texcmd = NULL;
    
    if (pdflatex_active()) typesetter = C_PDFLATEX;
    else typesetter = C_XELATEX;
    
    gchar *flags = texlive_get_flags("texpdf");
    
    gchar *dviname = g_strdup_printf("%s.dvi", basename);
    gchar *psname = g_strdup_printf("%s.ps", basename);
    
    if (utils_strequal (method, "texpdf")) {

        texcmd = g_strdup_printf("%s %s %s \"%s\"", typesetter, 
                                                flags,
                                                outdir, 
                                                workfile);
    }
    else if (utils_strequal (method, "texdvipdf")) {
        texcmd = g_strdup_printf("latex %s %s \"%s\" %s dvipdf -q \"%s\"",
                                                flags, 
                                                outdir, 
                                                workfile,
                                                C_CMDSEP,
                                                dviname);
    }
    else {
        texcmd = g_strdup_printf("latex %s %s \"%s\" %s dvips -q \"%s\" %s ps2pdf \"%s\"",
                                                flags, 
                                                outdir, 
                                                workfile,
                                                C_CMDSEP,
                                                dviname,
                                                C_CMDSEP,
                                                psname);
    }
    
    g_free(dviname);
    g_free(psname);
    
    return texcmd;
}

gchar* texlive_get_flags (const gchar* method) {
    gchar* defaults = g_strdup_printf("-interaction=nonstopmode "
                                      "-file-line-error "
                                      "-halt-on-error");
                                          
    if (!config_get_value("shellescape")) {
        gchar* optflags = g_strdup_printf("%s %s", 
                                      defaults, 
                                      "--no-shell-escape");
        return optflags;
    }
    return defaults;
}
