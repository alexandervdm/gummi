/**
 * @file   constants.h
 * @brief  Constants used throughout the program
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

#include "utils.h"

#ifndef __GUMMI_CONSTANTS_H__
#define __GUMMI_CONSTANTS_H__

// Gummi defs:
#define C_PACKAGE "gummi"
#define C_PACKAGE_NAME "Gummi"
#define C_PACKAGE_VERSION "0.8.3"
#define C_PACKAGE_COMMENTS "Simple LaTeX Editor for GTK+"
#define C_PACKAGE_COPYRIGHT "Copyright \xc2\xa9 2009\n\n"\
                            "Alexander van der Meij\n"\
                            "Wei-Ning Huang\n"\
                            "and past contributors"
#define C_PACKAGE_URL "https://gummi.app"
#define C_PACKAGE_GUIDE "https://github.com/alexandervdm/gummi/wiki"

// Project license:
#define C_PACKAGE_LICENSE \
"Copyright (C) 2009 Gummi Developers\n" \
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

// Credits:
#define C_CREDITS_DEVELOPERS \
"Alexander van der Meij", \
"Wei-Ning Huang", \
"", \
"with contributions by:", \
"Dion Timmermann", \
"Robert Schroll", \
"Thomas van der Burgt", \
"Cameron Grout", \
"Arnaud Loonstra", \
"Florian Begusch", \
"Daniel Hershcovich", \
"bobi32", \
"Benny Siegert", \
"scarlehoff"

#define C_CREDITS_DOCUMENTERS \
"Guy Edwards"

#define C_CREDITS_TRANSLATORS \
"Arabic: Hamad Mohammad\n" \
"Brazilian-Portugese: Fernando Cruz, Alexandre Guimarães\n" \
"Catalan: Marc Vinyals\n" \
"Chinese (Simplified): Mathlab pass, yjwork-cn\n" \
"Chinese (Traditional): Wei-Ning Huang\n" \
"Czech: Přemysl Janouch\n" \
"Danish: Jack Olsen\n" \
"Dutch: Alexander van der Meij\n" \
"French: Yvan Duron, Olivier Brousse\n" \
"German: Thomas Niederprüm\n" \
"Greek: Dimitris Leventeas\n" \
"Hungarian: Balázs Meskó\n" \
"Interlingue: OIS\n" \
"Italian: Salvatore Vassallo\n" \
"Polish: Hubert Kowalski\n" \
"Portugese: Alexandre Guimarães\n" \
"Romanian: Alexandru-Eugen Ichim\n" \
"Russian: Kruvalig, Max Musatov\n" \
"Swedish: Kess Vargavind\n" \
"Spanish: Carlos Salas Contreras, Francisco Javier Serrador\n"

// Default paths:
#define C_GUMMI_CONFDIR g_build_path (G_DIR_SEPARATOR_S, g_get_user_config_dir(), "gummi", NULL)
#define C_GUMMI_TEMPLATEDIR g_build_path (G_DIR_SEPARATOR_S, C_GUMMI_CONFDIR, "templates", NULL)

// Default documents:
#define C_WELCOMETEXT g_build_filename (C_GUMMI_CONFDIR, "welcome.tex", NULL)
#define C_DEFAULTTEXT g_build_filename (GUMMI_DATA, "misc", "default.tex", NULL)

// LaTeX definitions
#define C_LATEX "latex"
#define C_PDFLATEX "pdflatex"
#define C_XELATEX "xelatex"
#define C_LUALATEX "lualatex"
#define C_RUBBER "rubber"
#define C_LATEXMK "latexmk"

// Path definitions:
#define C_CD_TMPDIR g_strdup_printf ("cd \"%s\"%s%s",C_TMPDIR,C_CMDSEP,C_TEXSEC)
#define C_DIRSEP G_DIR_SEPARATOR_S

// Platform dependent path definitions:
#ifdef WIN32
    #define C_TMPDIR utils_get_tmp_tmp_dir()
    #define C_CMDSEP "&&"
    #define C_TEXSEC ""
#else
    #define C_TMPDIR g_build_path(G_DIR_SEPARATOR_S, g_get_user_cache_dir(), "gummi", NULL)
    #define C_CMDSEP ";"
    #define C_TEXSEC "env openout_any=a"
#endif

#endif /* __GUMMI_CONSTANTS_H__ */
