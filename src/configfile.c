/**
 * @file    configfile.c
 * @brief   handle configuration file
 *
 * Copyright (C) 2009 Gummi Developers
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

#include <glib.h>
#include <sys/stat.h>

#include "constants.h"
#include "utils.h"


const gchar default_config[] =
"[General]\n"
"config_version = "C_PACKAGE_VERSION"\n"
"\n"
"[Interface]\n"
"mainwindow_x = 0\n"
"mainwindow_y = 0\n"
"mainwindow_w = 792\n"
"mainwindow_h = 558\n"
"mainwindow_max = false\n"
"toolbar = true\n"
"statusbar = true\n"
"rightpane = true\n"
"snippets = true\n"
"[Editor]\n"
"font = Monospace 14\n"
"line_numbers = true\n"
"highlighting = true\n"
"textwrapping = true\n"
"wordwrapping = true\n"
"tabwidth = 4\n"
"spaces_instof_tabs = false\n"
"autoindentation = true\n"
"style_scheme = classic\n"
"spelling = false\n"
"spelling_lang = None\n"
"\n"
"[Preview]\n"
"zoom_mode = Fit Page Width\n"
"pagelayout = one_column\n"
"autosync = false\n"
"animated_scroll = always\n"
"cache_size = 150\n"
"\n"
"[File]\n"
"autosaving = false\n"
"autosave_timer = 10\n"
"autoexport = false\n"
"\n"
"[Compile]\n"
"typesetter = pdflatex\n"
"steps = texpdf\n"
"status = true\n"
"scheme = on_idle\n"
"timer = 1\n"
"shellescape = true\n"
"synctex = false\n"
"\n"
"[Misc]\n"
"recent1 = __NULL__\n"
"recent2 = __NULL__\n"
"recent3 = __NULL__\n"
"recent4 = __NULL__\n"
"recent5 = __NULL__\n";

GKeyFile *key_file = NULL;
gchar *conf_filepath = 0;


void config_init () {
    gchar* template_path;

    conf_filepath = g_build_filename (C_GUMMI_CONFDIR, "gummi.ini", NULL);
    template_path = g_build_path (G_DIR_SEPARATOR_S, C_GUMMI_CONFDIR, "templates", NULL);

    // create config & template dirs if not exists:
    if (!g_file_test (template_path, G_FILE_TEST_IS_DIR)) {
        slog (L_WARNING, "Template directory does not exist, creating..\n");
        g_mkdir_with_parents (template_path, DIR_PERMS);
    }
    g_free (template_path);

    // load config file:
    g_autoptr(GError) error = NULL;
    key_file = g_key_file_new ();

    if (!g_key_file_load_from_file (key_file, conf_filepath, G_KEY_FILE_NONE, &error)) {
        if (g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT)) {
            slog (L_WARNING, "Unable to load config, resetting defaults\n");
        }
        else {
            slog (L_ERROR, "%s\n", error->message);
        }
        config_load_defaults (key_file);
    }

    // replace old welcome texts if still active:
    gchar* text;
    if (g_file_get_contents (C_WELCOMETEXT, &text, NULL, NULL)) {
        guint hash = g_str_hash (text); // thank you mr daniel bernstein

        if (hash == -676241409) { // 0.6.6 default text - or unhappy collision
            slog (L_WARNING, "Replacing unchanged welcome text with new default\n");
            utils_copy_file (C_DEFAULTTEXT, C_WELCOMETEXT, NULL);
        }
    }
    g_free (text);

    slog (L_INFO, "Configuration file: %s\n", conf_filepath);
}

const gchar* config_get_string (const gchar* group, const gchar* key) {
    g_autoptr(GError) error = NULL;
    gchar* value;
    
    value = g_key_file_get_string (key_file, group, key, &error);

    if (error) {
        return config_get_default_string (group, key);
    }
    return value;
}

const gboolean config_get_boolean (const gchar* group, const gchar* key) {
    g_autoptr(GError) error = NULL;
    gboolean value = FALSE;

    value = g_key_file_get_boolean (key_file, group, key, &error);

    if (error) {
        return config_get_default_boolean (group, key);
    }
    return value;
}

const gint config_get_integer (const gchar* group, const gchar* key) {
    g_autoptr(GError) error = NULL;
    gint value = FALSE;

    value = g_key_file_get_integer (key_file, group, key, &error);

    if (error) {
        return config_get_default_integer (group, key);
    }
    return value;
}

const gchar* config_get_default_string (const gchar* group, const gchar* key) {
    g_autoptr(GKeyFile) default_keys = g_key_file_new ();
    gchar *default_value;

    g_key_file_load_from_data (default_keys, 
                               default_config, 
                               strlen (default_config), 
                               G_KEY_FILE_NONE, NULL);

    slog (L_WARNING, "Config get default value for '%s.%s'\n", group, key);

    default_value = g_key_file_get_string (default_keys, group, key, NULL);
    config_set_string (group, key, default_value);

    return default_value;
}

const gboolean config_get_default_boolean (const gchar* group, const gchar* key) {
    g_autoptr(GKeyFile) default_keys = g_key_file_new ();
    gboolean default_value;
    
    g_key_file_load_from_data (default_keys, 
                               default_config, 
                               strlen (default_config), 
                               G_KEY_FILE_NONE, NULL);

    slog (L_WARNING, "Config get default value for '%s.%s'\n", group, key);
    
    default_value = g_key_file_get_boolean (default_keys, group, key, NULL);
    config_set_boolean (group, key, default_value);
    
    return default_value;
}

const gint config_get_default_integer (const gchar* group, const gchar* key) {
    g_autoptr(GKeyFile) default_keys = g_key_file_new ();
    gint default_value;

    g_key_file_load_from_data (default_keys,
                               default_config,
                               strlen (default_config),
                               G_KEY_FILE_NONE, NULL);

    slog (L_WARNING, "Config get default value for '%s.%s'\n", group, key);

    default_value = g_key_file_get_integer (default_keys, group, key, NULL);
    config_set_integer (group, key, default_value);

    return default_value;
}

gboolean config_value_as_str_equals (const gchar* group, const gchar* key, gchar* input) {
    const gchar* value;

    value = config_get_string (group, key);
    if STR_EQU (value, input) {
        return TRUE;
    }
    return FALSE;
}

void config_set_string (const gchar *group, const gchar *key, gchar* value) {
    g_key_file_set_string (key_file, group, key, value);
}

void config_set_boolean (const gchar *group, const gchar *key, gboolean value) {
    g_key_file_set_boolean (key_file, group, key, value);
}

void config_set_integer (const gchar *group, const gchar *key, gint value) {
    g_key_file_set_integer (key_file, group, key, value);
}

void config_load_defaults () {
    g_autoptr(GError) error = NULL;

    g_key_file_load_from_data (key_file, default_config, strlen(default_config),
                               G_KEY_FILE_NONE, &error);

    if (error) {
        slog (L_ERROR, "Error loading default config: %s\n", error->message);
    }
    config_save ();
}

void config_save () {
    g_autoptr(GError) error = NULL;

    if (!g_key_file_save_to_file (key_file, conf_filepath, &error)) {
        if (error) {
            slog (L_ERROR, "Error saving config: %s\n", error->message);
        }
    }
}
