/**
 * @file    environment.h
 * @brief   
 *
 * Copyright (C) 2009-2012 Gummi-Dev Team <alexvandermey@gmail.com>
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

#define PACKAGE_COMMENTS "Simple LaTeX Editor for GTK+"
#define PACKAGE_COPYRIGHT "Copyright \xc2\xa9 2009-2012\n\n"\
                        "Alexander van der Mey\n"\
                        "Wei-Ning Huang\n"\
                        "Dion Timmermann\n"\
                        "Robert Schroll"
#define PACKAGE_LICENSE \
"Copyright (C) 2009-2012 Gummi-Dev Team <alexvandermey@gmail.com>\n" \
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
#include "importer.h"
#include "iofunctions.h"
#include "latex.h"
#include "motion.h"
#include "snippets.h"
#include "tabmanager.h"
#include "template.h"
#include "project.h"

#include "gui/gui-main.h"

#define _(T) gettext(T)

/**
 * Gummi:
 *
 * Stores Gummi main context.
 */
#define GUMMI(x) ((Gummi*)x)
typedef struct _Gummi Gummi;

struct _Gummi {
    /*< private >*/
    GuEditor* editor;
    GuIOFunc* io;
    GuMotion* motion;
    GuLatex* latex;
    GuBiblio* biblio;
    GuTemplate* templ;
    GuSnippets* snippets;
    GuTabmanager* tabmanager;
    GuProject* project;
};

Gummi* gummi_init (GuMotion* mo, GuIOFunc* io, GuLatex* latex, GuBiblio* bib, 
                   GuTemplate* tpl, GuSnippets* snip, GuTabmanager* tabm, 
                   GuProject* proj);
GuEditor* gummi_new_environment (const gchar* filename);

/**
 * Following APIs is used to eliminate the need of exposing global Gummi to
 * non-GUI classes.
 * Please only use this functions if not avoidable.
 */
 
gboolean gummi_project_active (void); 
 
GummiGui* gummi_get_gui (void);
GuEditor* gummi_get_active_editor (void);
GuIOFunc* gummi_get_io (void);
GuMotion* gummi_get_motion (void);
GuLatex* gummi_get_latex (void);
GuBiblio* gummi_get_biblio (void);
GuTemplate* gummi_get_template (void);
GuSnippets* gummi_get_snippets (void);

GList* gummi_get_all_tabs (void);
GList* gummi_get_all_editors (void);

#endif /* GUMMI_ENVIRONMENT_H */
