/**
 * @file   biblio.c
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

#include <gtk/gtk.h>
#include <string.h> 
#include <sys/stat.h> 

#include "biblio.h"
#include "utils.h"
#include "motion.h"
#include "environment.h"

extern GuEditor* ec;

GuBiblio* biblio_init(GtkBuilder * builder) {
    GuBiblio* b = g_new0(GuBiblio, 1);
    b->progressbar =
        GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "bibprogressbar"));
    b->progressmon =
        GTK_ADJUSTMENT(gtk_builder_get_object(builder, "bibprogressmon"));
    b->list_biblios = 
        GTK_LIST_STORE(gtk_builder_get_object(builder, "list_biblios"));
    b->filenm_label = 
        GTK_LABEL(gtk_builder_get_object(builder, "bibfilenm"));
    b->refnr_label = 
        GTK_LABEL(gtk_builder_get_object(builder, "bibrefnr"));
    b->progressval = 0.0;
    b->filename = NULL;
    b->basename = NULL;
    b->dirname = NULL;
    return b;
}


gboolean biblio_detect_bibliography(GuEditor* ec) {
    gchar* content;
    gchar** result;
    GMatchInfo *match_info;
    GRegex* bib_regex;
    
    content = editor_grab_buffer(ec);
    bib_regex = g_regex_new("\\\\bibliography{([^{}]*)}", 0, 0, NULL);
    if (g_regex_match(bib_regex, content, 0, &match_info)) {
        result = g_match_info_fetch_all(match_info);
        if (result[1] &&
            0 == strncmp(result[1] + strlen(result[1]) -4, ".bib", 4) &&
            utils_path_exists(result[1])) {
            g_strfreev(result);
            g_free(content);
            g_match_info_free(match_info);
            g_regex_unref(bib_regex);
            return TRUE;
        }
        g_strfreev(result);
    }
    g_free(content);
    g_match_info_free(match_info);
    g_regex_unref(bib_regex);
    return FALSE;
}

gboolean biblio_compile_bibliography(GuBiblio* bc, GuMotion* mc) {
    gchar* command = g_strdup_printf("bibtex '%s'", mc->b_finfo->workfile);
    motion_update_workfile(mc);
    motion_update_auxfile(mc);
    pdata res = utils_popen_r(command);
    gtk_widget_set_tooltip_text(GTK_WIDGET(bc->progressbar), res.data);
    g_free(command);
    return !(strstr(res.data, "Database file #1") == NULL);
}

gboolean biblio_setup_bibliography(GuBiblio* b, GuEditor* ec) {
    gchar *bibpath;
    gchar *dst;
    
    dst = g_strconcat(g_get_tmp_dir(), G_DIR_SEPARATOR_S, b->basename, NULL);
    utils_copy_file(b->filename, dst);
    bibpath = g_strconcat(b->dirname, G_DIR_SEPARATOR_S, b->basename, NULL);
    editor_insert_bib(ec, bibpath);

    g_free(dst);
    g_free(bibpath);
    return TRUE;
}

gboolean biblio_check_valid_file(GuBiblio* b, gchar *filename) {
    if (b->filename) g_free(b->filename);
    if (b->basename) g_free(b->basename);
    if (b->dirname) g_free(b->dirname);

    if (utils_path_exists(filename) == TRUE) {
        b->filename = g_strdup(filename);
        if (g_path_is_absolute(filename)) {
            b->basename = g_path_get_basename(filename);
            b->dirname = g_path_get_dirname(filename);
            return TRUE;
        }
        else {
            b->basename = g_strdup(filename);
            b->dirname = g_strdup(g_get_current_dir());
            return TRUE;
        }
    }
    else {
        return FALSE;
    }
}

int biblio_parse_entries(GuBiblio* bc, gchar *bib_content) {
    int entry_total = 0;
    
    GtkTreeIter iter;
    GRegex* regex_entry;
    GRegex* subregex_ident;
    GRegex* subregex_title;
    GRegex* subregex_author;
    GRegex* subregex_year;
    GRegex* regex_formatting;

    gchar* author_out;
    gchar* title_out;

    GMatchInfo *match_entry; 
    
    regex_entry = g_regex_new(
        "(@article|@book|@booklet|@conference|@inbook|@incollection|"
        "@inproceedings|@manual|@mastersthesis|@misc|@phdthesis|"
        "@proceedings|@techreport|@unpublished)([^@]*)", 
        (G_REGEX_CASELESS | G_REGEX_DOTALL), 0, NULL);
      
    subregex_ident = g_regex_new("@.+{([^,]+),", 0, 0, NULL);
    subregex_title = g_regex_new("[^book]title[\\s]*=[\\s]*(.*)", 0, 0, NULL);
    subregex_author = g_regex_new("author[\\s]*=[\\s]*(.*)", 0, 0, NULL);
    subregex_year = g_regex_new("year[\\s]*=[\\s]*[{|\"]?([1|2][0-9]{3})", 0,
            0, NULL);
    regex_formatting = g_regex_new("[{|}|\"|,|\\$]", 0, 0, NULL);
    
    
    g_regex_match(regex_entry, bib_content, 0, &match_entry);
    
    while (g_match_info_matches(match_entry)) {
        
        gchar *entry = g_match_info_fetch (match_entry, 0);

        gchar **ident_res = g_regex_split(subregex_ident, entry, 0);
        gchar **title_res = g_regex_split(subregex_title, entry, 0);
        gchar **author_res = g_regex_split(subregex_author, entry, 0);
        gchar **year_res = g_regex_split(subregex_year, entry, 0);

        author_out = g_regex_replace(regex_formatting, author_res[1],
                                     -1, 0, "", 0, 0);
        title_out = g_regex_replace(regex_formatting, title_res[1],
                                     -1, 0, "", 0, 0);
        
        gtk_list_store_append(bc->list_biblios, &iter);
        gtk_list_store_set(bc->list_biblios, &iter, 0, ident_res[1],
                                                    1, title_out,
                                                    2, author_out,
                                                    3, year_res[1], -1);
        g_free(author_out);
        g_free(title_out);
        g_strfreev(ident_res);
        g_strfreev(title_res);
        g_strfreev(author_res);
        g_strfreev(year_res);
        
        ++entry_total;
        g_free (entry);
        g_match_info_next (match_entry, NULL);
    }
    g_match_info_free(match_entry);
    g_regex_unref(regex_entry);
    g_regex_unref(subregex_ident);
    g_regex_unref(subregex_title);
    g_regex_unref(subregex_author);
    g_regex_unref(subregex_year);
    g_regex_unref(regex_formatting);
    
    return entry_total;
}
