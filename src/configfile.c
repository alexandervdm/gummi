/**
 * @file    configfile.c
 * @brief   handle configuration file
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <glib.h>

#include "environment.h"
#include "utils.h"

static gchar* config_filename = 0;

const gchar config_str[] =
"[Global]\n"
"config_version = "PACKAGE_VERSION"\n"
"mainwindow_x = 0\n"
"mainwindow_y = 0\n"
"mainwindow_w = 792\n"
"mainwindow_h = 558\n"
"\n"
"[Editor]\n"
"line_numbers = True\n"
"highlighting = True\n"
"textwrapping = True\n"
"wordwrapping = True\n"
"tabwidth = 8\n"
"spaces_instof_tabs = False\n"
"autoindentation = True\n"
"spelling = False\n"
"toolbar = True\n"
"statusbar = True\n"
"rightpane = True\n"
"spell_language = None\n"
"font = Monospace 10\n"
"\n"
"[File]\n"
"autosaving = False\n"
"autosave_timer = 10\n"
"\n"
"[Compile]\n"
"typesetter = pdflatex\n"
"compile_status = True\n"
"compile_scheme = on_idle\n"
"compile_timer = 1\n"
"\n"
"[Misc]\n"
"recent1 = __NULL__\n"
"recent2 = __NULL__\n"
"recent3 = __NULL__\n"
"recent4 = __NULL__\n"
"recent5 = __NULL__\n"
"welcome = \\documentclass{article}\n"
"	\\begin{document}\n"
"	\\begin{center}\n"
"	\\Huge{Welcome to Gummi} \\\\\\\n"
"	\\\\\n"
"	\\LARGE{You are using the "PACKAGE_VERSION" version.\\\\\n"
"	I welcome your suggestions at\\\\\n"
"	http://gummi.midnightcoding.org}\\\\\n"
"	\\end{center}\n"
"	\\end{document}\n\n";

void config_init(const gchar* filename) {
    L_F_DEBUG;
    const gchar* config_version = 0;
    gchar* dirname = 0;
    gchar buf[BUFSIZ] = { 0 };

    g_mkdir_with_parents(dirname = g_path_get_dirname(filename),
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    g_free(dirname);

    FILE* fh;
    if ((fh = fopen(filename, "r"))) {
        fgets(buf, BUFSIZ, fh);
        fclose(fh);
    }

    if (config_filename) g_free(config_filename);
    config_filename = g_strdup(filename);
    config_version = config_get_value("config_version");

    /* config_version field is not in gummi.cfg before 0.5.0 */
    if (0 == config_version[0]) {
        slog(L_INFO, "found old configuration file, replacing it with new "
                "one ...\n");
        config_set_default();
    }
}

void config_set_default(void) {
    L_F_DEBUG;
    FILE* fh = 0;
    if (!(fh = fopen(config_filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    fwrite(config_str, strlen(config_str), 1, fh);
    fclose(fh);
}

const gchar* config_get_value(const gchar* term) {
    L_F_DEBUG;
    FILE* fh = 0;
    gchar buf[BUF_MAX];
    static gchar ret[BUF_MAX];
    gchar* pstr;

    /* reset ret */
    ret[0] = 0;

    if (!(fh = fopen(config_filename, "r"))) {
        slog(L_ERROR, "can't find configuration file, reseting to default\n");
        config_set_default();
        return config_get_value(term);
    }

    while (fgets(buf, BUF_MAX, fh)) {
        buf[strlen(buf) -1] = 0;
        if (NULL == (pstr = strtok(buf, "[=] ")))
            continue;

        if (0 != strcmp(pstr, term))
            continue;
        if (NULL == (pstr = strtok(NULL, "=")))
            continue;
        else {
            strncpy(ret, pstr + 1, BUF_MAX);
            while (!feof(fh)) {
                fgets(buf, BUF_MAX, fh);
                buf[strlen(buf) -1] = 0;
                if (buf[0] == '\t') {
                    strncat(ret, "\n", BUF_MAX - strlen(ret) -1);
                    strncat(ret, buf + 1, BUF_MAX - strlen(ret) -1);
                } else break;
            }
            break;
        }
    }
    fclose(fh);

    if (NULL == ret)
        slog(L_ERROR, "can't find configuration for %s\n", term);

    if (0 == strcmp(ret, "False"))
        return NULL;
    return ret;
}

void config_set_value(const gchar* term, const gchar* value) {
    L_F_DEBUG;
    int i = 0, count = 0;
    int max = strlen(value) > BUF_MAX -1? BUF_MAX -1: strlen(value);
    slist* head = config_load();
    slist* index = 0;
    slist* current = 0;
    gchar buf[BUF_MAX];

    index = config_find_index_of(head, term);

    index->line[strlen(term) + 3] = 0;
    for (i = 0; i < max; ++i) {
        if (count == BUF_MAX -2) break;
        buf[count++] = value[i];
        if (value[i] == '\n')
            buf[count++] = '\t';
    }
    buf[count] = 0;

    strncat(index->line, buf, BUF_MAX - strlen(index->line) -2);
    strncat(index->line, "\n", BUF_MAX - strlen(index->line) -1);

    current = index->next;
    while (current) {
        if (current->line[0] == '\t')
            current->line[0] = 0;
        else break;
        current = current->next;
    }

    config_save(head);

    current = head;
    while (current) {
        index = current->next;
        g_free(current);
        current = index;
    }
}

slist* config_load(void) {
    L_F_DEBUG;
    FILE* fh = 0;
    slist* head = g_new0(slist, 1);
    slist* current = head;

    if (!(fh = fopen(config_filename, "r"))) {
        slog(L_ERROR, "can't find configuration file, reseting to default\n");
        config_set_default();
        return config_load();
    }

    while (!feof(fh)) {
        fgets(current->line, BUF_MAX, fh);
        current->next = g_new0(slist, 1);
        current = current->next;
    }
    fclose(fh);
    return head;
}

void config_save(slist* head) {
    L_F_DEBUG;
    FILE* fh = 0;
    slist* current = head;

    if (!(fh = fopen(config_filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    while (current) {
        if (strlen(current->line))
            fputs(current->line, fh);
        current = current->next;
    }
    fclose(fh);
}

slist* config_find_index_of(slist* head, const gchar* term) {
    /* return the index of the entry, if the entry does not exist, create a
     * new entry for it and return the new pointer. */
    L_F_DEBUG;
    slist* current = head;
    slist* prev = 0;

    while (current) {
        if (0 == strncmp(current->line, term, strlen(term)))
            return current;
        prev = current;
        current = current->next;
    }
    slog(L_WARNING, "can't find option `%s', creating new field for it...\n",
           term);
    prev->next = g_new0(slist, 1);
    current = prev->next;
    strncpy(current->line, term, BUF_MAX);
    strncat(current->line, " = __NULL__", BUF_MAX - strlen(current->line) - 1);
    return current;
}
