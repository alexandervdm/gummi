/**
 * @file    environment.h
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

#ifndef GUMMI_ENVIRONMENT_H
#define GUMMI_ENVIRONMENT_H

#ifdef HAVE_CONFIG_H
#   include "config.h"
#else
#   define PACKAGE "gummi"
#   define PACKAGE_NAME "Gummi"
#   define PACKAGE_VERSION "svn"
#   define PACKAGE_URL "http://gummi.midnightcoding.org/"
#endif

#define PACKAGE_COMMENTS "Simple LaTeX Editor for GTK+ users"
#define PACKAGE_COPYRIGHT "Copyright \xc2\xa9 2009 Alexander van der Mey"
#define PACKAGE_LICENSE \
"Copyright (C) 2010 Gummi-Dev Team <alexvandermey@gmail.com>\n" \
"All Rights reserved.\n" \
"\n" \
"Permission is hereby granted, free of charge, to any person\n" \
"obtaining a copy of this software and associated documentation\n" \
"files (the \"Software\"), to deal in the Software without\n" \
"restriction, including without limitation the rights to use,\n" \
"copy, modify, merge, publish, distribute, sublicense, and/or sell\n" \
"copies of the Software, and to permit persons to whom the\n" \
"Software is furnished to do so, subject to the following\n" \
"conditions:\n" \
"\n" \
"The above copyright notice and this permission notice shall be\n" \
"included in all copies or substantial portions of the Software.\n" \
"\n" \
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n" \
"EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\n" \
"OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n" \
"NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n" \
"HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n" \
"WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n" \
"FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n" \
"OTHER DEALINGS IN THE SOFTWARE.\n"

#include <glib.h>
#include <libintl.h>

#include "biblio.h"
#include "editor.h"
#include "environment.h"
#include "fileinfo.h"
#include "gummi-gui.h"
#include "importer.h"
#include "iofunctions.h"
#include "motion.h"
#include "preview.h"
#include "template.h"

#define _(T) gettext(T)

/* Gummi Class Hierarchy
 *
 * GuEditor GuFileInfo GuPreview          GuPrefsGui GuImportGui GuSearchGui
 *   |\     /    |      |  /                    \          |          /
 *   | `---+- GuMotion -+-`                      `----- GummiGui ----`
 *   |     |            |                                  |
 *   |     |            |   GuImporter GuTemplate GuBiblio |
 *   |     |            |        |          |        |     |
 *   |     |            |        |          |        |     |
 *   |     |            |        |          |        |     |
 *   `-----`------------`--------|----------`--------------`
 *                             Gummi
 */

typedef struct _Gummi {
    GummiGui* gui;
    GuFileInfo* finfo;
    GuEditor* editor;
    GuImporter* importer;
    GuMotion* motion;
    GuPreview* preview;
    GuBiblio* biblio;
    GuTemplate* templ;
} Gummi;

Gummi* gummi_init(GummiGui* gu, GuFileInfo* fc, GuEditor* ed, GuImporter* im,
        GuMotion* mo, GuPreview* prev, GuBiblio* bib, GuTemplate* tpl);
void gummi_create_environment(Gummi* gc, gchar* filename);

#endif /* GUMMI_ENVIRONMENT_H */
