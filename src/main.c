/**
 * @file   gummi.c
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

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "biblio.h"
#include "configfile.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "iofunctions.h"
#include "motion.h"
#include "snippets.h"
#include "template.h"
#include "utils.h"

static int debug = 0;
Gummi* gummi = 0;
GummiGui* gui = 0;

static GOptionEntry entries[] = {
    { (const gchar*)"debug", (gchar)'d', 0, G_OPTION_ARG_NONE, &debug, 
        (gchar*)"show debug info", NULL},
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

void on_window_destroy (GtkObject *object, gpointer user_data) {
    gtk_main_quit();
}

int main (int argc, char *argv[]) {
    gchar* configname;
    GtkBuilder* builder;
    GuFileInfo* finfo;
    GuEditor* editor;
    GuMotion* motion;
    GuLatex* latex;
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
    configname = g_build_filename(g_get_user_config_dir(), "gummi",
                                  "gummi.cfg", NULL);
    config_init(configname);
    config_load();
    g_free(configname);

    /* initialize gtk */
    gtk_init (&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, DATADIR"/gummi.glade", NULL);
    gtk_builder_set_translation_domain(builder, PACKAGE);

    /* initialize classes */
    gui = gui_init(builder);
    finfo = fileinfo_init();
    editor = editor_init(builder, finfo);
    latex = latex_init(finfo, editor); 
    biblio = biblio_init(builder, finfo);
    templ = template_init(builder, finfo);
    motion = motion_init(editor);

    gummi = gummi_init(finfo, editor, motion, latex, biblio, templ);

    slog_set_gui_parent(gui->mainwindow);

    if (argc != 2) {
        iofunctions_load_default_text(editor);
        gui_new_environment(NULL);
    } else {
        iofunctions_load_file(editor, argv[1]);
        gui_new_environment(argv[1]);
    }

    gui_main(builder);
    config_save();
    config_clean_up();
    return 0;
}
