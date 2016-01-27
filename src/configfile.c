/**
 * @file    configfile.c
 * @brief   handle configuration file
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <glib.h>
#include <assert.h>

#include "environment.h"
#include "utils.h"

static gchar* config_filename = 0;
static slist* config_head = 0;

static gboolean transaction = FALSE;

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
"tabwidth = 4\n"
"spaces_instof_tabs = False\n"
"autoindentation = True\n"
"style_scheme = classic\n"
"spelling = False\n"
"snippets = True\n"
"toolbar = True\n"
"statusbar = True\n"
"rightpane = True\n"
"spell_language = None\n"
"font = Monospace 10\n"
"\n"
"[Preview]\n"
"zoommode = pagewidth\n"
"pagelayout = one_column\n"
"autosync = False\n"
"animated_scroll = always\n"
"cache_size = 150\n"
"\n"
"[File]\n"
"autosaving = False\n"
"autosave_timer = 10\n"
"autoexport = False\n"
"\n"
"[Compile]\n"
"typesetter = pdflatex\n"
"compile_steps = texpdf\n"
"compile_status = True\n"
"compile_scheme = on_idle\n"
"compile_timer = 1\n"
"\n"
"[CompileOpts]\n"
"shellescape = True\n"
"synctex = False\n"
"\n"
"[Misc]\n"
"recent1 = __NULL__\n"
"recent2 = __NULL__\n"
"recent3 = __NULL__\n"
"recent4 = __NULL__\n"
"recent5 = __NULL__\n";

void config_init (const gchar* filename) {
    const gchar* config_version = NULL;
    gchar* dirname = g_path_get_dirname (filename);

    g_mkdir_with_parents (dirname, DIR_PERMS);
    g_free (dirname);

    slog (L_INFO, "configuration file: %s\n", filename);

    g_free (config_filename);
    config_filename = g_strdup (filename);

    config_load ();
    config_version = config_get_value ("config_version");


    /* Migration from earlier version to newer version (first run only) */
    if (utils_subinstr("0.5", (gchar*)config_version, FALSE)) {
        const gchar* text = config_get_value ("welcome");
        gchar* templname = g_strdup_printf("oldwelcome%s", config_version);
        gchar* filename = g_build_filename (g_get_user_config_dir (),
            "gummi", "templates", templname, NULL);
        gchar* filepath = g_path_get_dirname (filename);

        if (!g_file_test (filepath, G_FILE_TEST_IS_DIR)) {
            slog (L_WARNING, "Template directory does not exist, creating..\n");
            g_mkdir_with_parents (filepath, DIR_PERMS);
        }
        if (!g_file_test (filename, G_FILE_TEST_EXISTS) &&
            utils_set_file_contents (filename, text, -1)) {
            slog (L_WARNING, "Old welcome text successfully backed up..\n");
        }
        else {
            slog (L_WARNING, "Could not backup old welcome text..\n");
        }
        g_free (templname);
        g_free (filename);
        g_free (filepath);
    }

    /* config_version field is not in gummi.cfg before 0.5.0 */
    if (0 == config_version[0]) {
        slog (L_INFO, "found old configuration file, replacing it with new "
                "one ...\n");
        config_set_default ();
    } else if (!STR_EQU (PACKAGE_VERSION, config_version)) {
        slog (L_INFO, "updating version tag in configuration file...\n");
        config_set_value ("config_version", PACKAGE_VERSION);
    } else {
        config_save ();
    }
}

void config_set_default (void) {
    FILE* fh = 0;
    if (! (fh = fopen (config_filename, "w")))
        slog (L_FATAL, "can't open config for writing... abort\n");

    fwrite (config_str, strlen (config_str), 1, fh);
    fclose (fh);
    config_load ();
}

/**
 *  Returns the String assigned to a config parameter.
 *  If the String is "False", NULL is returned, thus in the case of a boolean
 *  parameter it is possible to write
 *      if (config_get_value("parameter_name")) {...}
 */
const gchar* config_get_value (const gchar* term) {
    gchar* ret  = NULL;
    slist* index = slist_find (config_head, term, FALSE, TRUE);

    ret = index->second;
    if (ret && STR_EQU (ret, "False"))
        return NULL;
    return ret;
}

void config_set_value (const gchar* term, const gchar* value) {
    if (!config_head)
        slog (L_FATAL, "configuration not initialized\n");

    slist* index = slist_find (config_head, term, FALSE, TRUE);
    g_free (index->second);

    index->second = g_strdup (value? value: "");
    if (!transaction)
        config_save ();
}

void config_begin (void) {
    transaction = TRUE;
}

void config_commit (void) {
    transaction = FALSE;
    config_save();
}

void config_load (void) {
    FILE* fh = 0;
    gchar buf[BUFSIZ];
    gchar** seg = NULL;
    slist* current = NULL;
    slist* prev = NULL;

    if (config_head)
        config_clean_up ();

    if (! (fh = fopen (config_filename, "r"))) {
        slog (L_ERROR, "can't find configuration file, reseting to default\n");
        config_set_default ();
        return config_load ();
    }

    /* In the occurence of a crash, the config file can be left open and the
     * contents wiped. Gummi segfaults at the start when the file is empty */
    gchar* contents;
    g_file_get_contents(config_filename, &contents, NULL, NULL);
    if (strlen(contents) == 0) {
        slog (L_ERROR, "config file appears empty, reseting to default\n");
        config_set_default ();
        return config_load ();
    }

    current = config_head = prev = g_new0 (slist, 1);

    while (fgets (buf, BUFSIZ, fh)) {
        buf[strlen (buf) -1] = 0; /* remove trailing '\n' */
        seg = g_strsplit(buf, "=", 2);
        if (seg[0]) {
            current->first = g_strdup (g_strstrip(seg[0]));
            current->second = g_strdup (seg[1]? g_strstrip(seg[1]): NULL);
        } else {
            current->first = g_strdup("");
            current->second = NULL;
        }
        g_strfreev(seg);
        prev = current;
        current->next = g_new0 (slist, 1);
        current = current->next;
    }
    g_free (current);
    prev->next = NULL;
    fclose (fh);
}

void config_save (void) {
    FILE* fh = 0;
    slist* current = config_head;

    if (! (fh = fopen (config_filename, "w")))
        slog (L_FATAL, "can't open config for writing... abort\n");

    while (current) {
        fputs (current->first, fh);
        if (current->second) {
            fputs (" = ", fh);
            fputs (current->second, fh);
        }
        fputs ("\n", fh);
        current = current->next;
    }
    fclose (fh);
}

void config_clean_up (void) {
    slist* prev = config_head;
    slist* current;
    while (prev) {
        current = prev->next;
        g_free (prev);
        prev = current;
    }
    config_head = NULL;
}
