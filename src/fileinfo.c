/**
 * @file    fileinfo.c
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

#include <unistd.h>

#include "fileinfo.h"
#include "utils.h"

GuFileInfo* fileinfo_init(void) {
    L_F_DEBUG;
    GuFileInfo* f = (GuFileInfo*)g_malloc(sizeof(GuFileInfo));
    f->workfd = -1;
    f->filename = NULL;   /* current opened file name in workspace */
    f->pdffile = NULL;
    f->workfile = NULL;
    f->tmpdir = g_get_tmp_dir();
    return f;
}

void fileinfo_set_filename(GuFileInfo* fc, const gchar* name) {
    L_F_DEBUG;
    if (fc->filename) g_free(fc->filename);
    if (name)
        fc->filename = g_strdup(name);
    else
        fc->filename = NULL;
}

void fileinfo_update(GuFileInfo* fc, const gchar* filename) {
    L_F_DEBUG;
    gchar* tname = NULL;

    if (fc->workfd != -1)
        fileinfo_destroy(fc);

    fileinfo_set_filename(fc, filename);

    tname = g_strdup_printf("%s%cgummi_XXXXXXX", fc->tmpdir, G_DIR_SEPARATOR);
    fc->workfd = g_mkstemp(tname); 

    if (fc->workfile) g_free(fc->workfile);
    fc->workfile = g_strdup(tname);

    if (fc->pdffile) g_free(fc->pdffile);
    fc->pdffile =  g_strdup_printf("%s.pdf", tname);
    g_free(tname);
}

void fileinfo_destroy(GuFileInfo* fc) {
    L_F_DEBUG;
    close(fc->workfd);
    // TODO: remove tempfiles 
}
