/**
 * @file    environment.c
 * @brief
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

#include "configfile.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "utils.h"

Gummi* gummi = 0;
GummiGui* gui = 0;

Gummi* gummi_init (GuMotion* mo, GuIOFunc* io, GuLatex* latex, GuBiblio* bib,
                   GuTemplate* tpl, GuSnippets* snip, GuTabmanager* tabm,
                   GuProject* proj) {

    Gummi* g = g_new0 (Gummi, 1);
    g->io = io;
    g->motion = mo;
    g->latex = latex;
    g->biblio = bib;
    g->templ = tpl;
    g->snippets = snip;
    g->tabmanager = tabm;
    g->project = proj;

    return g;
}

gboolean gummi_project_active (void) {
    if (gummi->project->projfile) return TRUE;
    return FALSE;
}

gchar* gummi_get_projectfile (void) {
    return gummi->project->projfile;
}

GuEditor* gummi_new_environment (const gchar* filename) {
    GuEditor* ec = editor_new (gummi->motion);
    editor_fileinfo_update (ec, filename);

    slog (L_INFO, "\n");
    slog (L_INFO, "Environment created for:\n");
    slog (L_INFO, "TEX: %s\n", ec->filename);
    slog (L_INFO, "TMP: %s\n", ec->workfile);
    slog (L_INFO, "PDF: %s\n", ec->pdffile);
    return ec;
}

GummiGui* gummi_get_gui (void) {
    return gui;
}

GuEditor* gummi_get_active_editor (void) {
    return g_active_editor;
}

GList* gummi_get_all_tabs (void) {
    return gummi->tabmanager->tabs;
}

GList* gummi_get_all_editors (void) {
    int tabtotal, i;
    GuEditor* ec;
    GList* editors = NULL;

    GList *tabs = gummi_get_all_tabs();
    tabtotal = g_list_length(tabs);

    for (i = 0; i < tabtotal; ++i) {
        ec = GU_TAB_CONTEXT (g_list_nth_data (tabs, i))->editor;
        editors = g_list_append (editors, ec);
    }
    return editors;
}

GuIOFunc* gummi_get_io (void) {
    return gummi->io;
}

GuMotion* gummi_get_motion (void) {
    return gummi->motion;
}

GuLatex* gummi_get_latex (void) {
    return gummi->latex;
}

GuBiblio* gummi_get_biblio (void) {
    return gummi->biblio;
}

GuTemplate* gummi_get_template (void) {
    return gummi->templ;
}

GuSnippets* gummi_get_snippets (void) {
    return gummi->snippets;
}

