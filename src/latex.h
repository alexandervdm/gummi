/**
 * @file    latex.h
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

#ifndef GUMMI_LATEX_H
#define GUMMI_LATEX_H

#include <glib.h>

#include "editor.h"
#include "fileinfo.h"
#include "gui/gui-preview.h"

typedef struct _GuLatex {
    GuFileInfo* b_finfo;
    GuEditor* b_editor;

    gchar* typesetter;
    gint errorline;
    gint prev_errorline;
    gchar* errormessage;
    gboolean modified_since_compile;
} GuLatex;

GuLatex* latex_init(GuFileInfo* fc, GuEditor* ec);
void latex_update_workfile(GuLatex* mc);
void latex_update_pdffile(GuLatex* mc);
void latex_update_auxfile(GuLatex* mc);
void latex_export_pdffile(GuLatex* mc, const gchar* path);

#endif /* GUMMI_LATEX_H */

