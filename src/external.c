/**
 * @file   external.c
 * @brief  existence and compability checks for external tools
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
 
#include "external.h"

#include "constants.h"
#include "utils.h"

/* local functions */
static gchar* version_texlive (const gchar* output);
static gchar* version_latexmk (const gchar* output);
static gchar* version_rubber (const gchar* output);

gboolean external_exists (const gchar* program) {
    const gchar *fullpath = g_find_program_in_path (program);
    
    if (g_file_test (fullpath, G_FILE_TEST_EXISTS)) {
        return TRUE;
    }
    return FALSE;
}

gboolean external_hasflag (const gchar* program, const gchar* flag) {
    return TRUE;
}

gchar* external_version (const gchar* program) {
    const gchar* getversion = g_strdup_printf("%s --version", program);
    Tuple2 cmdgetv = utils_popen_r (getversion);
    gchar* output = (gchar*)cmdgetv.second;
    gchar* result = g_strdup ("Unknown, please report a bug");
    
    if (output == NULL) return result;
    
    gchar** lines = g_strsplit(output, "\n", BUFSIZ);
    result = lines[0];
    
    /* the output for some programs needs tweaking, use local functions */
    if (utils_strequal (program, C_LATEX)) {
        result = version_texlive (result);
    }
    else if (utils_strequal (program, C_RUBBER)) {
        result = version_rubber (result);
    }
    else if (utils_strequal (program, C_LATEXMK)) {
        result = version_latexmk (lines[1]);
    }
    
    return result;
}

static gchar* version_texlive (const gchar* output) {
    gchar** parts = g_strsplit(output, " ", BUFSIZ);
    /* if we start splitting the year number from this string, please keep
     * in mind that debian/ubuntu like themselves a lot: (TeX Live 2009/Debian)
     * */
    gchar* version = g_strdup_printf("%s %s %s", parts[2], parts[3], parts[4]);
    return version;
}

static gchar* version_rubber (const gchar* output) {
    /* format: Rubber version: 1.1 */
    gchar** version = g_strsplit (output, " ", BUFSIZ);
    return version[2];
}

static gchar* version_latexmk (const gchar* output) {
    /* latexmk --version seems to print the requested information after a \n
       format: Latexmk, John Collins, 24 March 2011. Version 4.23a */

    gchar** version = g_strsplit (output, " ", BUFSIZ);
    return version[7];
}

    
