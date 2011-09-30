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
#include "utils.h"

gboolean lmk_detected = FALSE;

void latexmk_init (void) {
    
    if (utils_program_exists (C_LATEXMK)) {
        // TODO: check if supported version
        slog (L_INFO, "Typesetter detected: %s\n", utils_get_version (C_LATEXMK));
        lmk_detected = TRUE;
    }
}

gboolean latexmk_active (void) {
    if (utils_strequal (config_get_value("typesetter"), C_LATEXMK)) {
        return TRUE;
    }
    return FALSE;
}

gboolean latexmk_detected (void) {
    return lmk_detected;
}

gchar* latexmk_get_command (const gchar* method, gchar* workfile) {
    
    const gchar* flags = latexmk_get_flags (method);
    gchar* lmkcmd;
    
    lmkcmd = g_strdup_printf("latexmk %s \"%s\"", flags, workfile);
    return lmkcmd;
    
    /*
    const gchar* outdir = g_strdup_printf ("--into=\"%s\"", C_TMPDIR);
    const gchar* flags = rubber_get_flags (method);
    gchar* rubcmd;
    
    rubcmd = g_strdup_printf("rubber %s %s \"%s\"", flags, outdir, workfile);
    
    return rubcmd;
    */
}

gchar* latexmk_get_flags (const gchar *method) {
    gchar *lmkflags;
    if (utils_strequal (method, "texpdf")) {
        lmkflags = g_strdup_printf("-pdf");
    }
    else if (utils_strequal (method, "texdvipdf")){
        lmkflags = g_strdup_printf("-pdfdvi");
    }
    else {
        lmkflags = "-pdfps";
    }
    return lmkflags;
}




