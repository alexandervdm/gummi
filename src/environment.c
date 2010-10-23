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
#include "gui-main.h"
#include "utils.h"

Gummi* gummi_init(GummiGui* gu, GuFileInfo* fc, GuEditor* ed, GuImporter* im,
        GuMotion* mo, GuPreview* prev, GuBiblio* bib, GuTemplate* tpl) {
    L_F_DEBUG;
    Gummi* g = g_new0(Gummi, 1);
    g->gui = gu;
    g->finfo = fc;
    g->editor = ed;
    g->importer = im;
    g->motion = mo;
    g->preview = prev;
    g->biblio = bib;
    g->templ = tpl;
    return g;
}

void gummi_create_environment(Gummi* gc, gchar* filename) {
    L_F_DEBUG;

    fileinfo_update(gc->finfo, filename);
    slog(L_INFO, "\n");
    slog(L_INFO, "Environment created for:\n");
    slog(L_INFO, "TEX: %s\n", gc->finfo->filename);
    slog(L_INFO, "TMP: %s\n", gc->finfo->workfile);
    slog(L_INFO, "PDF: %s\n", gc->finfo->pdffile); 

    /* Title will be updated in motion_update_pdffile */
    gui_update_title();

    /* This is important */
    gc->motion->errorline = 1;
    motion_initial_preview(gc->motion);

    if (!gc->motion->errorline && config_get_value("compile_status"))
        motion_start_updatepreview(gc->motion);
    if (config_get_value("autosaving"))
        iofunctions_reset_autosave(filename);
}
