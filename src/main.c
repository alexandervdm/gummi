/**
 * @file   gummi.c
 * @brief
 *
 * Copyright (C) 2009-2012 Gummi-Dev Team <alexvandermey@gmail.com>
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
#include "external.h"
#include "gui/gui-main.h"
#include "iofunctions.h"
#include "motion.h"
#include "project.h"
#include "signals.h"
#include "snippets.h"
#include "tabmanager.h"
#include "template.h"
#include "utils.h"

extern Gummi* gummi;
extern GummiGui* gui;
static int debug = 0;

static GOptionEntry entries[] = {
    { (const gchar*)"debug", (gchar)'d', 0, G_OPTION_ARG_NONE, &debug, 
        (gchar*)"show debug info", NULL},
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

void on_window_destroy (GtkObject *object, gpointer user_data) {
    gtk_main_quit ();
}

int main (int argc, char *argv[]) {
    /* set up i18n */
    bindtextdomain (PACKAGE, LOCALEDIR);
    setlocale (LC_ALL, "");
    textdomain (PACKAGE);

    GError* error = NULL;
    GOptionContext* context = g_option_context_new ("files");
    g_option_context_add_main_entries (context, entries, PACKAGE);
    g_option_context_parse (context, &argc, &argv, &error);
    if (error) g_error("%s\n", error->message);

    /* initialize GTK */
    g_thread_init (NULL);
    gdk_threads_init ();
    gtk_init (&argc, &argv);
    GError* ui_error = NULL;
    GtkBuilder* builder = gtk_builder_new ();
    gchar* ui = g_build_filename (DATADIR, "ui", "gummi.glade", NULL);
    
    // exit program when gummi.glade can not be located:
    if (!g_file_test (ui, G_FILE_TEST_EXISTS)) {
        printf("Could not locate Glade interface file at:\n%s\n", ui);
        return 0;
    }
    
    gtk_builder_add_from_file (builder, ui, &ui_error);
    if (ui_error) {
        g_error ("%s\n", ui_error->message);
    }
    gtk_builder_set_translation_domain (builder, PACKAGE);
    g_free (ui);

    /* Initialize logging */
    slog_init (debug);
    slog (L_INFO, PACKAGE_NAME" version: "PACKAGE_VERSION"\n");

    /* Initialize configuration */
    gchar* configname = g_build_filename (g_get_user_config_dir (), "gummi",
                                  "gummi.cfg", NULL);
    config_init (configname);
    config_load ();
    g_free (configname);

    /* Initialize signals */
    gummi_signals_register ();

    /* Initialize Classes */
    gchar* snippetsname = g_build_filename (g_get_user_config_dir (), "gummi",
            "snippets.cfg", NULL);

    // why do we not load this from environment, like gui-main does? -A
    GuMotion* motion = motion_init ();
    GuIOFunc* io = iofunctions_init();
    GuLatex* latex = latex_init (); 
    GuBiblio* biblio = biblio_init (builder);
    GuTemplate* templ = template_init (builder);
    GuTabmanager* tabm = tabmanager_init ();
    GuProject* proj = project_init ();

    GuSnippets* snippets = snippets_init (snippetsname);
    gummi = gummi_init (motion, io, latex, biblio, templ, snippets, tabm, proj);
    slog (L_DEBUG, "Gummi created!\n");
    g_free (snippetsname);

    /* Initialize GUI */
    gui = gui_init (builder);
    
    slog_set_gui_parent (gui->mainwindow);
    slog (L_DEBUG, "GummiGui created!\n");

    /* Start compile thread */
    if (external_exists (config_get_value("typesetter"))) {
        typesetter_setup ();
        motion_start_compile_thread (motion);
    }
    else {
        infoscreengui_enable (gui->infoscreengui, "program_error");
        slog (L_ERROR, "Could not locate the typesetter program\n");
    }

    /* Install acceleration group to mainwindow */
    gtk_window_add_accel_group (gui->mainwindow, snippets->accel_group);

    if (argc != 2)
        tabmanager_create_tab (A_DEFAULT, NULL, NULL);
    else {
        if (!g_file_test(argv[1], G_FILE_TEST_EXISTS)) {
            slog(L_ERROR, "Failed to open file '%s': No such file or "
                    "directory\n", argv[1]);
            exit(1);
        }
        tabmanager_create_tab (A_LOAD, argv[1], NULL);
    }
        
    if (config_get_value ("autosaving")) iofunctions_start_autosave ();

    gui_main (builder);
    config_save ();
    config_clean_up ();
    return 0;
}
