/**
 * @file   external.c
 * @brief  existence and compatibility checks for external tools
 *
 * Copyright (C) 2009 Gummi Developers
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

#include "external.h"

#include "constants.h"
#include "utils.h"

/* local functions */
static gchar* get_version_output (const gchar* command, int linenr);
static gchar* version_latexmk (gchar* output);
static gchar* version_rubber (gchar* output);


static gdouble get_texlive_version (void);


gboolean external_exists (const gchar* program) {
    gchar *fullpath = g_find_program_in_path (program);
    if (fullpath == NULL) return FALSE;

    gboolean result = g_file_test (fullpath, G_FILE_TEST_EXISTS);
    g_free(fullpath);
    return result;
}

gboolean external_hasflag (const gchar* program, const gchar* flag) {
    return TRUE;
}

static gchar* get_version_output (const gchar* command, int linenr) {
    const gchar* getversion = g_strdup_printf("%s --version", command);
    Tuple2 cmdgetv = utils_popen_r (getversion, NULL);
    gchar* output = (gchar*)cmdgetv.second;
    gchar* result = g_strdup ("Unknown");

    if (output == NULL) {
        slog (L_ERROR, "Error detecting version for %s. "
                       "Please report a bug\n", command);
        return result;
    }

    gchar** split = g_strsplit(output, "\n", BUFSIZ);
    result = split[linenr];
    return result;
}

gdouble external_version2 (ExternalProg program) {

    switch(program) {
        case EX_TEXLIVE: return get_texlive_version ();
        default: return -1;
    }
}

gchar* external_version (const gchar* program) {
    gchar* version_output;
    gchar* result;

    const gchar* version_cmd = g_strdup_printf("%s --version", program);
    Tuple2 cmdgetv = utils_popen_r (version_cmd, NULL);
    version_output = (gchar*)cmdgetv.second;

    if (version_output == NULL || g_str_equal (version_output, "")) {
        return g_strdup_printf("Unknown, please report a bug");
    }
    else {
        result = g_strstrip (version_output);
    }

    /* pdfTeX 3.1415926-1.40.10 (TeX Live 2009)
       pdfTeX 3.1415926-1.40.11-2.2 (TeX Live 2010)
       pdfTeX 3.1415926-2.3-1.40.12 (TeX Live 2011)

       This is LuaTeX, Version 1.10.0 (TeX Live 2019/Debian)
    */
    if (STR_EQU (program, C_RUBBER)) {
        result = version_rubber (result);
    }
    else if (STR_EQU (program, C_LATEXMK)) {
        result = version_latexmk (result);
    }

    return result;
}

static gdouble get_texlive_version (void) {
    gdouble version = 0;
    gchar* output = get_version_output (C_LATEX, 0);

    /* Keep in mind that some distros like themselves a lot:
     * pdfTeX 3.1415926-1.40.11-2.2 (TeX Live 2010)
     * pdfTeX 3.1415926-1.40.11-2.2 (TeX Live 2009/Debian)
     * pdfTeX 3.1415926-2.3-1.40.12 (TeX Live 2012/dev/Arch Linux)
     * pdfTeX 3.1415926-2.3-1.40.12 (Web2C 2011)
     *
     * Also, TeXLive utilities from versions before 2008 do not
     * mention the year in the --version tag. */

    if ((!utils_subinstr ("TeX Live", output, FALSE)) &&
        (!utils_subinstr ("Web2C", output, FALSE))) {
        return version;
    }

    gchar** split = g_strsplit (output, "(", BUFSIZ);
    guint size = g_strv_length (split);

    gchar* segment = g_strdup (split[size-1]);
    segment = g_strjoinv("", g_strsplit(segment, "Web2C", -1));
    gchar* resultstr = "";

    // make sure to only allow numeric characters in the result:
    int n;
    for (n=0;n<g_utf8_strlen(segment, -1);n++) {
        if (g_ascii_isdigit (segment[n])) {
           gchar* addchar = g_strdup_printf("%c", segment[n]);
           resultstr = g_strconcat (resultstr, addchar, NULL);
        }
    }

    version = g_ascii_strtod (resultstr, NULL);
    return version;
}

static gchar* version_rubber (gchar* output) {
    // format: Rubber version: 1.1
    gchar** outarr = g_strsplit (output, " ", BUFSIZ);
    gchar* version = g_strdup (outarr [g_strv_length(outarr) - 1]);

    g_strfreev (outarr);
    return version;
}

static gchar* version_latexmk (gchar* output) {
    // format: Latexmk, John Collins, 24 March 2011. Version 4.23a
    gchar** outarr = g_strsplit (output, " ", BUFSIZ);
    gchar* version = g_strdup (outarr [g_strv_length(outarr) - 1]);

    g_strfreev (outarr);
    return version;
}
