/**
 * @file    iofunctions.c
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


#include "iofunctions.h"

#include <iconv.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "configfile.h"
#include "editor.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "utils.h"

extern Gummi* gummi;
static guint sid = 0;

void iofunctions_load_default_text(GuEditor* ec) {
    gchar* str = g_strdup(config_get_value("welcome"));
    editor_fill_buffer(ec, str);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(ec->sourcebuffer), FALSE);
    g_free(str);
}

void iofunctions_load_file(GuEditor* ec, const gchar* filename) {
    GError* err = NULL;
    gchar* status;
    gchar* text;
    gchar* decoded;
    gboolean result;

    slog(L_INFO, "loading %s ...\n", filename);

    /* add Loading message to status bar and ensure GUI is current */
    status = g_strdup_printf ("Loading %s...", filename);
    statusbar_set_message(status);
    g_free(status);
    while (gtk_events_pending()) gtk_main_iteration();
    
    /* get the file contents */
    if (FALSE == (result = g_file_get_contents(filename, &text, NULL, &err))) {
        slog(L_G_ERROR, "g_file_get_contents(): %s\n", err->message);
        g_error_free(err);
        iofunctions_load_default_text(ec);
        goto cleanup;
    }
    if (NULL == (decoded = iofunctions_decode_text(text)))
        goto cleanup;

    editor_fill_buffer(ec, decoded);
    gtk_text_buffer_set_modified(ec_sourcebuffer, FALSE);

cleanup:
    g_free(decoded);
    g_free(text); 
}

void iofunctions_write_file(GuEditor* ec, const gchar* filename) {
    GError* err = NULL;
    gchar* status;
    gchar* text;
    gchar* encoded;
    gboolean result;

    status = g_strdup_printf(_("Saving %s..."), filename);
    statusbar_set_message(status);    
    g_free (status);
    while (gtk_events_pending()) gtk_main_iteration();
    
    text = editor_grab_buffer(ec);
    encoded = iofunctions_encode_text(text);
    
    /* set the contents of the file to the text from the buffer */
    if (filename != NULL) {
        if (!(result = g_file_set_contents (filename, text, -1, &err))) {
            slog(L_ERROR, "g_file_set_contents(): %s\n", err->message);
            g_error_free(err);
        }
    }
        
    if (result == FALSE) {
        slog(L_G_ERROR, _("%s\nPlease try again later."), err->message);
        g_error_free(err);
    }
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(ec->sourcebuffer), FALSE);
    g_free(encoded);
    g_free(text); 
}

void iofunctions_start_autosave(const gchar* name) {
    static gchar* filename = NULL;
    if (filename) {
        g_free(filename);
        filename = NULL;
    }
    filename = g_strdup(name);
    sid = g_timeout_add_seconds(atoi(config_get_value("autosave_timer")) * 60,
            iofunctions_autosave_cb, filename);
}

void iofunctions_stop_autosave(void) {
    if (sid > 0) g_source_remove(sid);
}

void iofunctions_reset_autosave(const gchar* name) {
    iofunctions_stop_autosave();
    if (config_get_value("autosaving"))
        iofunctions_start_autosave(name);
}

char* iofunctions_decode_text(gchar* text) {
    GError* err = NULL;
    gchar* result = 0;
    gsize read = 0, written = 0;

    if (!(result = g_locale_to_utf8(text, -1, &read, &written, &err))) {
        g_error_free(err);
        slog(L_ERROR, "failed to convert text from default locale, trying "
                "ISO-8859-1\n");
        gsize in_size = strlen(text), out_size = in_size * 2;
        gchar* out = (gchar*)g_malloc(out_size);
        gchar* process = out;
        iconv_t cd = iconv_open("UTF-8//IGNORE", "ISO−8859-1");

        if (-1 == iconv(cd, &text, &in_size, &process, &out_size)) {
            slog(L_G_ERROR, _("Can not convert text to UTF-8!\n"));
            g_free(out);
            out = NULL;
        }
        result = out;
    }
    return result;
}

gchar* iofunctions_encode_text(gchar* text) {
    GError* err = NULL;
    gchar* result = 0;
    gsize read = 0, written = 0;

    if (!(result = g_locale_from_utf8(text, -1, &read, &written, &err))) {
        g_error_free(err);
        slog(L_ERROR, "failed to convert text to default locale, text will "
                "be saved in UTF-8\n");
        result = g_strdup(text);
    }
    return result;
}

gboolean iofunctions_autosave_cb(gpointer name) {
    char* fname = (char*)name;
    char* buf = g_strdup_printf(_("Autosaving file %s"), fname);
    if (fname) {
        iofunctions_write_file(gummi->editor, fname);
        gtk_text_buffer_set_modified(
                GTK_TEXT_BUFFER(gummi->editor->sourcebuffer), FALSE);
        statusbar_set_message(buf);
        g_free(buf);
        return TRUE;
    }
    g_free(buf);
    return FALSE;
}
