/**
 * @file    environment.c
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

#include "configfile.h"
#include "environment.h"
#include "utils.h"

Gummi* gummi_init(GList* eds, GuMotion* mo, GuLatex* latex, GuBiblio* bib,
        GuTemplate* tpl, GuSnippets* snip) {
    Gummi* g = g_new0(Gummi, 1);
    g->editors = eds;
    g->editor = g_list_first(eds)->data;
    g->motion = mo;
    g->latex = latex;
    g->biblio = bib;
    g->templ = tpl;
    g->snippets = snip;
    return g;
}

void gummi_new_environment(Gummi* gc, const gchar* filename) {
    editor_fileinfo_update(gc->editor, filename);

    slog(L_INFO, "\n");
    slog(L_INFO, "Environment created for:\n");
    slog(L_INFO, "TEX: %s\n", gc->editor->filename);
    slog(L_INFO, "TMP: %s\n", gc->editor->workfile);
    slog(L_INFO, "PDF: %s\n", gc->editor->pdffile); 

    iofunctions_reset_autosave(filename);
}
