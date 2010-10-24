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
slist* config_head = 0;

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
"	\\end{document}\n";

void config_init(const gchar* filename) {
    L_F_DEBUG;
    const gchar* config_version = NULL;
    gchar* dirname = NULL;

    g_mkdir_with_parents(dirname = g_path_get_dirname(filename),
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    g_free(dirname);

    slog(L_INFO, "configuration file: %s\n", filename);

    g_free(config_filename);
    config_filename = g_strdup(filename);

    config_load();
    config_version = config_get_value("config_version");

    /* config_version field is not in gummi.cfg before 0.5.0 */
    if (0 == config_version[0]) {
        slog(L_INFO, "found old configuration file, replacing it with new "
                "one ...\n");
        config_set_default();
    } else if (0 != strcmp(PACKAGE_VERSION, config_version)) {
        slog(L_INFO, "updating version tag in configuration file...\n");
        config_set_value("config_version", PACKAGE_VERSION);
    }
    config_save();
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
    gchar* ret  = NULL;
    slist* index = config_find_index_of(config_head, term);

    ret = index->line + strlen(term) + 3; /* strlen(" = ") = 3 */

    if (0 == strcmp(ret, "False"))
        return NULL;
    return ret;
}

void config_set_value(const gchar* term, const gchar* value) {
    L_F_DEBUG;
    if (!config_head)
        slog(L_FATAL, "configuration not initialized\n");

    slist* index = config_find_index_of(config_head, term);
    g_free(index->line);
    index->line = g_strconcat(term, " = ", value, NULL);
}

void config_load(void) {
    L_F_DEBUG;
    FILE* fh = 0;
    gchar buf[BUFSIZ];
    gchar* rot = NULL;
    slist* current = config_head;
    slist* prev = current;

    if (config_head) {
        while (prev) {
            current = prev->next;
            g_free(prev);
            prev = current;
        }
        config_head = NULL;
    }

    if (!(fh = fopen(config_filename, "r"))) {
        slog(L_ERROR, "can't find configuration file, reseting to default\n");
        config_set_default();
        return config_load();
    }

    current = config_head = prev = g_new0(slist, 1);

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

void config_save(void) {
    L_F_DEBUG;
    FILE* fh = 0;
    slist* current = config_head;
    gint i = 0, count = 0, len = 0;
    gchar* buf = 0;

    if (!(fh = fopen(config_filename, "w")))
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
    current->line = g_strconcat(term, " = __NULL__", NULL);
    return current;
}
