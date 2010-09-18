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

static const gchar* config_filename = 0;
static const gchar* templcfg_filename = 0;

const gchar configfile_str[] =
"[Global]\n"
"configfile_version = "PACKAGE_VERSION"\n"
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
"autosave_timer = 600\n"
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
"	\\LARGE{You are using the "PACKAGE_VERSION" version.\n"
"	This package is a NIGHTLY BUILD as part of the\\\\\n"
"	\\textbf{Gummi "PACKAGE_VERSION"}.\\\\\n"
"	This package is NOT FOR PRODUCTION USE.\n"
"	We would appreciate reports on any problems/bugs you experience using this package.\n"
"	Please run your Gummi from the command line with the \\textit{\"gummi-beta -d\"} command.\\\\\\\n"
"	\\\\\n"
"	I welcome your suggestions at\\\\\n"
"	http://gummi.midnightcoding.org}\\\\\n"
"	\\end{center}\n"
"	\\end{document}\n\n";

void configfile_init(const gchar* filename, gint type) {
    L_F_DEBUG;
    const gchar* configfile_version = 0;
    gchar buf[BUFSIZ] = { 0 };

    g_mkdir_with_parents(g_path_get_dirname(filename),
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    FILE* fh;
    if ((fh = fopen(filename, "r"))) {
        fgets(buf, BUFSIZ, fh);
        fclose(fh);
    }
    
    if (type == 0) { /* gummi.cfg */
        config_filename = filename;
        configfile_version = configfile_get_value(type, "configfile_version");

        if (!configfile_version ||
                0 != strcmp(configfile_version, PACKAGE_VERSION) ||
                0 == strcmp(buf, "[DEFAULT]\n")) {
            slog(L_INFO, "found old configuration file, replacing it with new "
                    "one ...\n");
            configfile_set_default(type);
        }
    } else {
        templcfg_filename = filename;
        /* dummy action to check if template.cfg exist */
        configfile_get_value(type, "template");
    }
}

void configfile_set_default(gint type) {
    L_F_DEBUG;
    FILE* fh = 0;
    if (!(fh = fopen(CONFIG_NAME(type), "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    if (type == 0)
        fwrite(configfile_str, strlen(configfile_str), 1, fh);

    fclose(fh);
}

const gchar* configfile_get_value(gint type, const gchar* term) {
    L_F_DEBUG;
    FILE* fh = 0;
    gchar buf[BUF_MAX];
    static gchar ret[BUF_MAX];
    gchar* pstr;

    /* reset ret */
    ret[0] = 0;

    if (!(fh = fopen(CONFIG_NAME(type), "r"))) {
        slog(L_ERROR, "can't find configuration file, reseting to default\n");
        configfile_set_default(type);
        return configfile_get_value(type, term);
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

void configfile_set_value(gint type, const gchar* term, const gchar* value) {
    L_F_DEBUG;
    int i = 0, count = 0;
    int max = strlen(value) > BUF_MAX -1? BUF_MAX -1: strlen(value);
    slist* head = configfile_load(type);
    slist* index = 0;
    slist* current = 0;
    gchar buf[BUF_MAX];

    index = configfile_find_index_of(head, term);

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
    }

    configfile_save(type, head);

    current = head;
    while (current) {
        index = current->next;
        g_free(current);
        current = index;
    }
}

slist* configfile_load(gint type) {
    L_F_DEBUG;
    FILE* fh = 0;
    slist* head = g_new0(slist, 1);
    slist* current = head;

    if (!(fh = fopen(CONFIG_NAME(type), "r"))) {
        slog(L_ERROR, "can't find configuration file, reseting to default\n");
        configfile_set_default(type);
        return configfile_load(type);
    }

    while (!feof(fh)) {
        fgets(current->line, BUF_MAX, fh);
        current->next = g_new0(slist, 1);
        current = current->next;
    }
    fclose(fh);
    return head;
}

void configfile_save(gint type, slist* head) {
    L_F_DEBUG;
    FILE* fh = 0;
    slist* current = head;

    if (!(fh = fopen(CONFIG_NAME(type), "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    while (current) {
        if (strlen(current->line))
            fputs(current->line, fh);
        current = current->next;
    }
    fclose(fh);
}

slist* configfile_find_index_of(slist* head, const gchar* term) {
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
    prev->next = g_new0(slist, 1);
    current = prev->next;
    strncpy(current->line, term, BUF_MAX);
    strncat(current->line, " = __NULL__", BUF_MAX - strlen(current->line) - 1);
    return current;
}
