/**
 * @file   biblio.c
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

#include <gtk/gtk.h>
#include <string.h>
#include <sys/stat.h>

#include "biblio.h"
#include "constants.h"
#include "utils.h"
#include "latex.h"
#include "environment.h"

extern GuEditor* ec;

GuBiblio* biblio_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuBiblio* b = g_new0 (GuBiblio, 1);
    b->progressbar =
        GTK_PROGRESS_BAR (gtk_builder_get_object (builder, "bibprogressbar"));
    b->progressmon =
        GTK_ADJUSTMENT (gtk_builder_get_object (builder, "bibprogressmon"));
    b->list_biblios =
        GTK_LIST_STORE (gtk_builder_get_object (builder, "list_biblios"));
    b->filenm_label =
        GTK_LABEL (gtk_builder_get_object (builder, "bibfilenm"));
    b->refnr_label =
        GTK_LABEL (gtk_builder_get_object (builder, "bibrefnr"));
    b->list_filter =
        GTK_ENTRY (gtk_builder_get_object (builder, "biblio_filter"));
    b->biblio_treeview =
        GTK_TREE_VIEW (gtk_builder_get_object (builder, "bibtreeview"));
    b->progressval = 0.0;
    return b;
}

gboolean biblio_detect_bibliography (GuBiblio* bc, GuEditor* ec) {
    gchar* content = NULL;
    gchar* bibfn = NULL;
    gchar** result = NULL;
    GMatchInfo *match_info;
    GRegex* bib_regex = NULL;
    gboolean state = FALSE;

    content = editor_grab_buffer (ec);
    bib_regex = g_regex_new ("^[^%]*\\\\bibliography{\\s*([^{}\\s]*)\\s*}",
        G_REGEX_MULTILINE, 0, NULL);
    if (g_regex_match (bib_regex, content, 0, &match_info)) {
        result = g_match_info_fetch_all (match_info);
        if (result[1]) {
            if (!STR_EQU (result[1] +strlen (result[1]) -4, ".bib"))
                bibfn = g_strconcat (result[1], ".bib", NULL);
            else
                bibfn = g_strdup (result[1]);
            state = editor_fileinfo_update_biblio (ec, bibfn);
            g_free (bibfn);
        }
        slog (L_INFO, "Detect bibliography file: %s\n", ec->bibfile);
        g_strfreev (result);
    }
    g_free (content);
    g_match_info_free (match_info);
    g_regex_unref (bib_regex);
    return state;
}

gboolean biblio_compile_bibliography (GuBiblio* bc, GuEditor* ec, GuLatex* lc) {
    gchar* dirname = g_path_get_dirname (ec->workfile);
    gchar* auxname = NULL;

    if (ec->filename) {
        auxname = g_strdup (ec->pdffile);
        auxname[strlen (auxname) -4] = 0;
    } else
        auxname = g_strdup (ec->fdname);

    if (g_find_program_in_path ("bibtex")) {
        gboolean success = FALSE;
        char* command = g_strdup_printf ("%s bibtex \"%s\"",
                                         C_TEXSEC, auxname);

        g_free (auxname);
        latex_update_workfile (lc, ec);
        latex_update_auxfile (lc, ec);
        Tuple2 res = utils_popen_r (command, dirname);
        gtk_widget_set_tooltip_text (GTK_WIDGET (bc->progressbar),
                (gchar*)res.second);
        g_free (command);
        g_free (dirname);
        success = ! (strstr ((gchar*)res.second, "Database file #1") == NULL);
        g_free (res.second);
        return success;
    }
    slog (L_WARNING, "bibtex command is not present or executable.\n");
    g_free (auxname);
    g_free (dirname);
    return FALSE;
}

int biblio_parse_entries (GuBiblio* bc, gchar *bib_content) {
    int entry_total = 0;

    GtkTreeIter iter;
    GRegex* regex_entry;
    GRegex* subregex_ident;
    GRegex* subregex_title;
    GRegex* subregex_author;
    GRegex* subregex_year;
    GRegex* regex_formatting;

    gchar* author_out = NULL;
    gchar* title_out = NULL;

    GMatchInfo *match_entry;

    regex_entry = g_regex_new (
        "(@article|@book|@booklet|@conference|@inbook|@incollection|"
        "@inproceedings|@manual|@mastersthesis|@misc|@phdthesis|"
        "@proceedings|@techreport|@unpublished)([^@]*)",
        (G_REGEX_CASELESS | G_REGEX_DOTALL), 0, NULL);

    subregex_ident = g_regex_new ("@.+{([^,]+),", 0, 0, NULL);
    subregex_title = g_regex_new
		("[^book]title[\\s]*=[\\s]*(.*)", G_REGEX_CASELESS, 0, NULL);
    subregex_author = g_regex_new
		("author[\\s]*=[\\s]*(.*)", G_REGEX_CASELESS, 0, NULL);
    subregex_year = g_regex_new
		("year[\\s]*=[\\s]*[{|\"]?([1|2][0-9]{3})", G_REGEX_CASELESS, 0, NULL);
    regex_formatting = g_regex_new ("[{|}|\"|,|\\$]", 0, 0, NULL);


    g_regex_match (regex_entry, bib_content, 0, &match_entry);

    while (g_match_info_matches (match_entry)) {

        gchar *entry = g_match_info_fetch (match_entry, 0);

        gchar **ident_res = g_regex_split (subregex_ident, entry, 0);
        gchar **title_res = g_regex_split (subregex_title, entry, 0);
        gchar **author_res = g_regex_split (subregex_author, entry, 0);
        gchar **year_res = g_regex_split (subregex_year, entry, 0);

        if (author_res[1])
            author_out = g_regex_replace (regex_formatting, author_res[1],
                                         -1, 0, "", 0, 0);
        else author_out = NULL;

        if (title_res[1])
            title_out = g_regex_replace (regex_formatting, title_res[1],
                                         -1, 0, "", 0, 0);
        else title_out = NULL;

        gtk_list_store_append (bc->list_biblios, &iter);
        gtk_list_store_set (bc->list_biblios, &iter, 0, ident_res[1],
                                                    1, title_out,
                                                    2, author_out,
                                                    3, year_res[1], -1);
        g_free (author_out);
        g_free (title_out);
        g_strfreev (ident_res);
        g_strfreev (title_res);
        g_strfreev (author_res);
        g_strfreev (year_res);
        g_free (entry);
        ++entry_total;

        g_match_info_next (match_entry, NULL);
    }
    g_match_info_free (match_entry);
    g_regex_unref (regex_entry);
    g_regex_unref (subregex_ident);
    g_regex_unref (subregex_title);
    g_regex_unref (subregex_author);
    g_regex_unref (subregex_year);
    g_regex_unref (regex_formatting);

    return entry_total;
}
