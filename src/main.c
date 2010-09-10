/**
 * @file   main.c
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

#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "configfile.h"
#include "environment.h"
#include "gui.h"
#include "importer.h"
#include "iofunctions.h"
#include "template.h"
#include "utils.h"
#include "biblio.h"

static int debug = 0;
Gummi* gummi = 0;

static GOptionEntry entries[] = {
    { (const gchar*)"debug", (gchar)'d', 0, G_OPTION_ARG_NONE, &debug, 
        (gchar*)"show debug info", NULL},
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

void on_window_destroy (GtkObject *object, gpointer user_data) {
    gtk_main_quit();
}

int main (int argc, char *argv[]) {
    gchar configname[128];
    GtkBuilder* builder;
    GummiGui* gui;
    GuFileInfo* finfo;
    GuEditor* editor;
    GuImporter* importer;
    GuMotion* motion;
    GuPreview* preview;
    GuTemplate* templ;
    GuBiblio* biblio;

    /* set up i18n */
    bindtextdomain(PACKAGE, LOCALEDIR);
    setlocale(LC_ALL, "");
    textdomain(PACKAGE);

    GError* error = NULL;
    GOptionContext* context = g_option_context_new("files");
    g_option_context_add_main_entries(context, entries, PACKAGE);
    g_option_context_parse(context, &argc, &argv, &error);

    slog_init(debug);
    slog(L_INFO, PACKAGE_NAME" version: "PACKAGE_VERSION"\n");

    /* set up configuration file */
    snprintf(configname, 128, "%s%cgummi", g_get_user_config_dir(),
            G_DIR_SEPARATOR);
    g_mkdir_with_parents(configname,
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    strncat(configname, G_DIR_SEPARATOR_S, 128 - strlen(configname) -1);
    strncat(configname, "gummi.cfg", 128 - strlen(configname) -1);
    config_init(configname);
    slog(L_INFO, "configuration file: %s\n", configname);
    config_get_value("wtf");

    /* initialize gtk */
    gtk_init (&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, DATADIR"/gummi.glade", NULL);
    gtk_builder_set_translation_domain(builder, PACKAGE);

    /* initialize classes */
    gui = gui_init(builder);
    finfo = fileinfo_init();
    editor = editor_init(builder);
    importer = importer_init(builder);
    preview = preview_init(builder);
    motion = motion_init(builder, finfo, editor, preview); 
    templ = template_init(builder);
    biblio = biblio_init(builder);

    gummi = gummi_init(gui, finfo, editor, importer, motion, preview, biblio,
            templ);

    slog_set_gui_parent(gui->mainwindow);

    if (argc != 2) {
        iofunctions_load_default_text(editor);
        gummi_create_environment(gummi, NULL);
    } else {
        iofunctions_load_file(editor, argv[1]);
        gummi_create_environment(gummi, argv[1]);
    }

    gui_main(builder);
    return 0;
}
