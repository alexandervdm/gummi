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

#include <glib.h>
#include <glib/gstdio.h>

#include "fileinfo.h"
#include "utils.h"

GuFileInfo* fileinfo_init(void) {
    L_F_DEBUG;
    GuFileInfo* f = g_new0(GuFileInfo, 1);
    f->workfd = -1;
    f->fdname = NULL;
    f->filename = NULL;   /* current opened file name in workspace */
    f->pdffile = NULL;
    f->workfile = NULL;
    f->bibfile = NULL;
    f->tmpdir = g_get_tmp_dir();
    return f;
}

/* FileInfo:
 * When a TeX document includes materials from other files(image, documents,
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
 * workfile is saved under DIRNAME(FILENAME). Other compilation-related files
 * are located in the temp directory.
 *
 * P.S. pdflatex will automatically strip the suffix, so for a file named
 * FILE.tex under /absolute/path/:
 *
 * filename = /absolute/path/FILE.tex
 * workfile = /absolute/path/.FILE.tex.swp
 * pdffile = /tmp/.FILE.tex.pdf
 */

void fileinfo_update(GuFileInfo* fc, const gchar* filename) {
    L_F_DEBUG;

    if (fc->workfd != -1)
        fileinfo_destroy(fc);

    fc->fdname = g_build_filename(fc->tmpdir, "gummi_XXXXXX", NULL);
    fc->workfd = g_mkstemp(fc->fdname); 

    if (filename) {
        gchar* basename = g_path_get_basename(filename);
        gchar* dirname = g_path_get_dirname(filename);
        fc->filename = g_strdup(filename);
        fc->workfile = g_strdup_printf("%s%c.%s.swp", dirname, G_DIR_SEPARATOR,
                                       basename);
        fc->pdffile =  g_strdup_printf("%s%c.%s.pdf", fc->tmpdir,
                                       G_DIR_SEPARATOR, basename);
        g_free(basename);
        g_free(dirname);
    } else {
        fc->workfile = g_strdup(fc->fdname);
        fc->pdffile =  g_strdup_printf("%s.pdf", fc->fdname);
    }
}

void fileinfo_destroy(GuFileInfo* fc) {
    L_F_DEBUG;
    gchar* auxfile = NULL;
    gchar* logfile = NULL;

    if (fc->filename) {
        gchar* dirname = g_path_get_dirname(fc->filename);
        gchar* basename = g_path_get_basename(fc->filename);
        auxfile = g_strdup_printf("%s%c.%s.aux", fc->tmpdir,
                G_DIR_SEPARATOR, basename);
        logfile = g_strdup_printf("%s%c.%s.log", fc->tmpdir,
                G_DIR_SEPARATOR, basename);
        g_free(basename);
        g_free(dirname);
    } else {
        gchar* dirname = g_path_get_dirname(fc->workfile);
        gchar* basename = g_path_get_basename(fc->workfile);
        auxfile = g_strdup_printf("%s.aux", fc->fdname);
        logfile = g_strdup_printf("%s.log", fc->fdname);
        g_free(basename);
        g_free(dirname);
    }

    close(fc->workfd);
    fc->workfd = -1;

    g_remove(auxfile);
    g_remove(logfile);
    g_remove(fc->fdname);
    g_remove(fc->workfile);
    g_remove(fc->pdffile);

    g_free(auxfile);
    g_free(logfile);
    g_free(fc->fdname);
    g_free(fc->filename);
    g_free(fc->workfile);
    g_free(fc->pdffile);

    fc->fdname = NULL;
    fc->filename = NULL;
    fc->workfile = NULL;
    fc->pdffile = NULL;
}
