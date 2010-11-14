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

#include "snippet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <glib.h>

#include "environment.h"
#include "utils.h"

static gchar* snippet_filename = 0;
slist* snippet_head = 0;

void snippet_init(const gchar* filename) {
    L_F_DEBUG;
    const gchar* snippet_version = NULL;
    gchar* dirname = NULL;

    g_mkdir_with_parents(dirname = g_path_get_dirname(filename),
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    g_free(dirname);

    slog(L_INFO, "snippets : %s\n", filename);

    g_free(snippet_filename);
    snippet_filename = g_strdup(filename);
}

void snippet_set_default(void) {
    L_F_DEBUG;
    FILE* fh = 0;
    if (!(fh = fopen(snippet_filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    fwrite(snippet_str, strlen(snippet_str), 1, fh);
    fclose(fh);
}

void snippet_load(void) {
    L_F_DEBUG;
    FILE* fh = 0;
    gchar buf[BUFSIZ];
    gchar* rot = NULL;
    slist* current = snippet_head;
    slist* prev = current;

    if (snippet_head) {
        while (prev) {
            current = prev->next;
            g_free(prev);
            prev = current;
        }
        snippet_head = NULL;
    }

    if (!(fh = fopen(snippet_filename, "r"))) {
        slog(L_ERROR, "can't find configuration file, reseting to default\n");
        snippet_set_default();
        return snippet_load();
    }

    current = snippet_head = prev = g_new0(slist, 1);

    while (fgets(buf, BUFSIZ -1, fh)) {
        buf[strlen(buf) -1] = 0; /* remove trailing '\n' */
        if (buf[0] != '\t') {
            current->line = g_strdup(buf);
        } else {
            rot = g_strdup(prev->line);
            g_free(prev->line);
            prev->line = g_strconcat(rot, "\n", buf + 1, NULL);
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

void snippet_save(void) {
    L_F_DEBUG;
    FILE* fh = 0;
    slist* current = snippet_head;
    gint i = 0, count = 0, len = 0;
    gchar* buf = 0;

    if (!(fh = fopen(snippet_filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    while (current) {
        len = strlen(current->line) + 1;
        buf = (gchar*)g_malloc(len * 2);
        memset(buf, 0, len * 2);
        /* replace '\n' with '\n\t' for options with multi-line content */
        for (i = 0; i < len; ++i) {
            if (count + 2 == len * 2) break;
            buf[count++] = current->line[i];
            if (i != len -2 && '\n' == current->line[i])
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
