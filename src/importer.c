/**
 * @file   importer.c
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

#include "importer.h"

#include <string.h>

#include <glib.h>

#include "editor.h"
#include "environment.h"
#include "utils.h"

extern Gummi* gummi;

const gchar align_type[][4] = { "l", "c", "r" };
const gchar bracket_type[][16] = { "matrix", "pmatrix", "bmatrix",
                                  "Bmatrix", "vmatrix", "Vmatrix" };

const gchar* importer_generate_table (gint rows, gint cols, gint borders,
        gint alignment) {
    gint i = 0, j = 0;
    static gchar result[BUFSIZ * 2] = { 0 };
    gchar table[BUFSIZ * 2] = { 0 },
          begin_tabular[BUFSIZ] = "\\begin{tabular}{",
          end_tabular[] = "\n\\end{tabular}\n",
          line[] = "\n\\hline",
          tmp[BUFSIZ / 8];

    /* clear previous data */
    result[0] = 0;

    if (borders)
        strncat (begin_tabular, "|", BUFSIZ - strlen (begin_tabular) -1);
    for (i = 0; i < cols; ++i) {
        strncat (begin_tabular, align_type[alignment], BUFSIZ
                -strlen (begin_tabular) -1);
        if (borders == 2 || (borders == 1 && i == cols -1))
            strncat (begin_tabular, "|", BUFSIZ -strlen (begin_tabular) -1);
    }
    strncat (begin_tabular, "}", BUFSIZ -strlen (begin_tabular) -1);
    if (borders)
        strncat (table, line, BUFSIZ * 2 -strlen (table) -1);
    for (i = 0; i < rows; ++i) {
        strncat (table, "\n\t", BUFSIZ * 2 -strlen (table) -1);
        for (j = 0; j < cols; ++j) {
            snprintf (tmp, BUFSIZ/8, "%d%d", i + 1, j + 1);
            strncat (table, tmp, BUFSIZ * 2 -strlen (table) -1);
            if (j != cols -1)
                strncat (table, " & ", BUFSIZ * 2 -strlen (table) -1);
            else
                strncat (table, "\\\\", BUFSIZ * 2 -strlen (table) -1);
        }
        if (borders == 2 || (borders == 1 && i == rows -1))
            strncat (table, line, BUFSIZ * 2 -strlen (table) -1);
    }
    strncat (result, begin_tabular, BUFSIZ *2 -strlen (result) -1);
    strncat (result, table, BUFSIZ *2 -strlen (result) -1);
    strncat (result, end_tabular, BUFSIZ *2 -strlen (result) -1);
    return result;
}

const gchar* importer_generate_matrix (gint bracket, gint rows, gint cols) {
    gint i = 0, j = 0;
    static gchar result[BUFSIZ * 2] = { 0 };
    gchar tmp[BUFSIZ / 8];

    /* clear previous data */
    result[0] = 0;

    strncat (result, "$\\begin{", BUFSIZ * 2 -strlen (result) -1);
    strncat (result, bracket_type[bracket], BUFSIZ * 2 -strlen (result) -1);
    strncat (result, "}", BUFSIZ * 2 - strlen (result) -1);

    for (i = 0; i < rows; ++i) {
        strncat (result, "\n\t", BUFSIZ * 2 -strlen (result) -1);
        for (j = 0; j < cols; ++j) {
            snprintf (tmp, BUFSIZ/8, "%d%d", i + 1, j + 1);
            strncat (result, tmp, BUFSIZ * 2 -strlen (result) -1);
            if (j != cols -1)
                strncat (result, " & ", BUFSIZ * 2 -strlen (result) -1);
            else
                strncat (result, "\\\\", BUFSIZ * 2 -strlen (result) -1);
        }
    }
    strncat (result, "\n\\end{", BUFSIZ * 2 -strlen (result) -1);
    strncat (result, bracket_type[bracket], BUFSIZ * 2 -strlen (result) -1);
    strncat (result, "}$\n", BUFSIZ * 2 -strlen (result) -1);
    return result;
}

const gchar* importer_generate_image (const gchar* path, const gchar* caption,
        const gchar* label, gdouble scale) {
    static gchar result[BUFSIZ] = { 0 };
    gchar scale_str[16] = { 0 };
    gchar* loc = NULL;

    /* clear previous data */
    result[0] = 0;

    // Filepath notation corrections for Windows systems:
    #ifdef WIN32
    path = g_strjoinv("/", g_strsplit(path, "\\", -1));
    if (utils_subinstr (" ", path, FALSE)) {
        editor_insert_package (g_active_editor, "grffile", "space");
    }
    #endif

    snprintf (scale_str, 16, "%.2f", scale);

    /* some locales use ',' as seperator, replace them as '.' */
    if ( (loc = strstr (scale_str, ",")))
        *loc = '.';

    snprintf (result, BUFSIZ, "\\begin{figure}[htp]\n\\centering\n"
        "\\includegraphics[scale=%s]{%s}\n\\caption{%s}\n\\label{%s}\n"
        "\\end{figure}", scale_str, path, caption, label);
    return result;
}
