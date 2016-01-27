/**
 * @file   constants.h
 * @brief  Constants used throughout the program
 *
 * Copyright (C) 2009-2016 Gummi Developers
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

/* File constants */
#define C_WELCOMETEXT g_build_filename ( g_get_user_config_dir (), "gummi", "welcome.tex", NULL)
#define C_DEFAULTTEXT g_build_filename (GUMMI_DATA, "misc", "default.tex", NULL)

/* URL constants */
#define C_GUMMIGUIDE "https://github.com/alexandervdm/gummi/wiki"


#define C_LATEX "latex"
#define C_PDFLATEX "pdflatex"
#define C_XELATEX "xelatex"
#define C_RUBBER "rubber"
#define C_LATEXMK "latexmk"

#define C_CD_TMPDIR g_strdup_printf ("cd \"%s\"%s%s",C_TMPDIR,C_CMDSEP,C_TEXSEC)
#define C_DIRSEP G_DIR_SEPARATOR_S




/* Platform dependant constants : */

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
