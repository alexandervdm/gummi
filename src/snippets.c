/**
 * @file    snippets.c
 * @brief   handle snppets
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

#include "snippets.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <glib.h>

#include "environment.h"
#include "utils.h"

GuSnippets* snippets_init(const gchar* filename, GuEditor* ec) {
    GuSnippets* s = g_new0(GuSnippets, 1);
    gchar* dirname = NULL;

    g_mkdir_with_parents(dirname = g_path_get_dirname(filename),
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    g_free(dirname);

    slog(L_INFO, "snippets : %s\n", filename);

    s->b_editor = ec;
    s->filename = g_strdup(filename);
    return s;
}

void snippets_set_default(GuSnippets* sc) {
    FILE* fh = 0;
    if (!(fh = fopen(sc->filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    //fwrite(snippets_str, strlen(snippets_str), 1, fh);
    fclose(fh);
}

void snippets_load(GuSnippets* sc) {
    FILE* fh = 0;
    gchar buf[BUFSIZ];
    gchar* rot = NULL;
    gchar* seg = NULL;
    slist* current = NULL;
    slist* prev = NULL;

    if (sc->head)
        snippets_clean_up(sc);

    if (!(fh = fopen(sc->filename, "r"))) {
        slog(L_ERROR, "can't find snippets file, reseting to default\n");
        snippets_set_default(sc);
        return snippets_load(sc);
    }

    current = sc->head = prev = g_new0(slist, 1);

    while (fgets(buf, BUFSIZ, fh)) {
        buf[strlen(buf) -1] = 0; /* remove trailing '\n' */
        if (buf[0] != '\t') {
            if ('#' == buf[0]) {
                current->first = g_strdup(buf);
            } else {
                seg = strtok(buf, " ");
                seg = strtok(NULL, " ");
                current->first = g_strdup((seg == buf)? "Invalid": seg);
            }
        } else {
            if (!prev->second) {
                prev->second = g_strdup(buf + 1);
                continue;
            }
            rot = g_strdup(prev->second);
            g_free(prev->second);
            prev->second = g_strconcat(rot, "\n", buf + 1, NULL);
            g_free(rot);
            g_free(current);
            current = prev;
        }
        prev = current;
        current->next = g_new0(slist, 1);
        current = current->next;
    }
    g_free(current);
    prev->next = NULL;
    fclose(fh);
}

void snippets_save(GuSnippets* sc) {
    FILE* fh = 0;
    slist* current = sc->head;
    gint i = 0, count = 0, len = 0;
    gchar* buf = 0;

    if (!(fh = fopen(sc->filename, "w")))
        slog(L_FATAL, "can't open snippets file for writing... abort\n");

    while (current) {
        /* skip comments */
        if ('#' == current->first[0]) {
            fputs(current->first, fh);
            fputs("\n", fh);
            current = current->next;
            continue;
        }
        fputs("snippet ", fh);
        fputs(current->first, fh);
        fputs("\n\t", fh);

        len = strlen(current->second) + 1;
        buf = (gchar*)g_malloc(len * 2);
        memset(buf, 0, len * 2);
        /* replace '\n' with '\n\t' for options with multi-line content */
        for (i = 0; i < len; ++i) {
            if (count + 2 == len * 2) break;
            buf[count++] = current->second[i];
            if (i != len -2 && '\n' == current->second[i])
                buf[count++] = '\t';
        }
        fputs(buf, fh);
        fputs("\n", fh);
        current = current->next;
        count = 0;
        g_free(buf);
    }
    fclose(fh);
}

void snippets_clean_up(GuSnippets* sc) {
    slist* prev = sc->head;
    slist* current;
    while (prev) {
        current = prev->next;
        g_free(prev);
        prev = current;
    }
    sc->head = NULL;
}
