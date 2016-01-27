/**
 * @file    biblio.h
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

#ifndef __GUMMI_BIBLIO_H__
#define __GUMMI_BIBLIO_H__

#include <gtk/gtk.h>

#include "latex.h"

#define GU_BIBLIO(x) ((GuBiblio*)x)
typedef struct _GuBiblio GuBiblio;

struct _GuBiblio {
    GtkProgressBar* progressbar;
    GtkAdjustment* progressmon;
    GtkListStore* list_biblios;
    GtkTreeView* biblio_treeview;
    GtkLabel* filenm_label;
    GtkLabel* refnr_label;
    GtkEntry* list_filter;
    gchar* basename;
    double progressval;
};

GuBiblio* biblio_init (GtkBuilder* builder);
gboolean biblio_detect_bibliography (GuBiblio* bc, GuEditor* ec);
gboolean biblio_compile_bibliography (GuBiblio* bc, GuEditor* ec, GuLatex* lc);
int biblio_parse_entries (GuBiblio* bc, gchar *bib_content);


#endif /* __GUMMI_BIBLIO_H__ */
