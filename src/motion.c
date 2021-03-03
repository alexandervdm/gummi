/**
 * @file   motion.c
 * @brief
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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#ifndef WIN32
    #include <sys/types.h>
    #include <sys/wait.h>
#endif

#ifdef WIN32
    #include <windows.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include "configfile.h"
#include "editor.h"
#include "environment.h"
#include "gui/gui-main.h"
#include "gui/gui-preview.h"
#include "latex.h"
#include "snippets.h"
#include "utils.h"

extern GummiGui* gui;
extern Gummi* gummi;

/* Typesetter pid */
pid_t typesetter_pid = 0;

GuMotion* motion_init (void) {
    GuMotion* m = g_new0 (GuMotion, 1);

    m->key_press_timer = 0;
    g_mutex_init(&m->signal_mutex);
    g_mutex_init(&m->compile_mutex);
    g_cond_init(&m->compile_cv);
    m->keep_running = TRUE;
    m->keep_running = FALSE;
    m->typesetter_pid = &typesetter_pid;

    return m;
}

void motion_start_compile_thread (GuMotion* m) {
    m->keep_running = TRUE;
    m->compile_thread = g_thread_new ("motion", motion_compile_thread, m);
}

void motion_stop_compile_thread (GuMotion* m) {
    L_F_DEBUG;

    m->keep_running = FALSE;
    motion_do_compile(m);
    g_thread_join(m->compile_thread);
}

void motion_pause_compile_thread (GuMotion* m) {
    L_F_DEBUG;

    m->pause = TRUE;
    motion_do_compile(m);
}

void motion_resume_compile_thread (GuMotion* m) {
    L_F_DEBUG;

    m->pause = FALSE;
    motion_do_compile(m);
}

void motion_kill_typesetter (GuMotion* m) {
    if (*m->typesetter_pid) {
        /* Kill children spawned by typesetter command/script, don't know
         * how to do this programatically yet(glib doesn't not provides any
         * function for killing a process), so use pkill for now. For
         * win32 there's currently nothing we can do about it. */
#ifndef WIN32
        gchar* command = g_strdup_printf("pkill -15 -P %d", *m->typesetter_pid);
        system(command);
        g_free(command);

        /* Make sure typesetter command is terminated */
        if (kill(*m->typesetter_pid, 15)) {
            slog(L_ERROR, "Could not kill process: %s\n",
                                    g_strerror(errno));
        }
#else
        if (!TerminateProcess(*m->typesetter_pid, 0)) {
            gchar *msg = g_win32_error_message(GetLastError());
            slog (L_ERROR, "Could not kill process: %s\n",
                                    msg ? msg : "(null)");
            g_free(msg);
        }

#endif


        slog(L_DEBUG, "Typeseter[pid=%d]: Killed\n", *m->typesetter_pid);
        *m->typesetter_pid = 0;

        /* XXX: Ugly hack: delay compile signal */
        motion_start_timer (m);
    }
}

gboolean motion_do_compile (gpointer user) {
    L_F_DEBUG;
    GuMotion* mc = GU_MOTION (user);

    if (!g_mutex_trylock (&mc->signal_mutex)) goto ret;
    g_cond_signal (&mc->compile_cv);
    g_mutex_unlock (&mc->signal_mutex);

ret:
    return (config_value_as_str_equals ("Compile", "scheme", "real_time"));
}

gpointer motion_compile_thread (gpointer data) {
    L_F_DEBUG;
    GuMotion* mc = GU_MOTION (data);
    GuEditor* editor = NULL;
    GuLatex* latex = NULL;
    gboolean precompile_ok = FALSE;
    gchar *editortext;

    latex = gummi_get_latex ();

    while (TRUE) {
        if (!g_mutex_trylock (&mc->compile_mutex)) continue;
        slog (L_DEBUG, "Compile thread sleeping...\n");
        g_cond_wait (&mc->compile_cv, &mc->compile_mutex);
        slog (L_DEBUG, "Compile thread awoke.\n");

        if (!(editor = gummi_get_active_editor ())) {
            g_mutex_unlock (&mc->compile_mutex);
            continue;
        }
        if (!mc->keep_running) {
            g_mutex_unlock (&mc->compile_mutex);
            g_thread_exit (NULL);
        }

        if (mc->pause) {
            g_mutex_unlock (&mc->compile_mutex);
            continue;
        }

        gdk_threads_enter ();
        editortext = latex_update_workfile (editor);
        precompile_ok = latex_precompile_check (editortext);
        g_free (editortext);
        gdk_threads_leave ();

        if (!precompile_ok) {
            gdk_threads_add_idle (on_document_error, "document_error");
            g_mutex_unlock (&mc->compile_mutex);
            continue;
        }

        latex_update_pdffile (latex, editor);

        *mc->typesetter_pid = 0;
        g_mutex_unlock (&mc->compile_mutex);

        if (!mc->keep_running)
            g_thread_exit (NULL);

        gdk_threads_add_idle (on_document_compiled, editor);
        continue;
    }
}

void motion_force_compile (GuMotion *mc) {
    /* sort-of signal to force a compile run after certain actions that
     * don't trigger the regular editor content change signals */
    gummi->latex->modified_since_compile = TRUE;
    motion_do_compile (mc);
}

gboolean motion_idle_cb (gpointer user) {
    GU_MOTION(user)->key_press_timer = 0;
    if (gui->previewgui->preview_on_idle)
        motion_do_compile (GU_MOTION (user));
    return FALSE;
}

void motion_start_timer (GuMotion* mc) {
    motion_stop_timer (mc);
    mc->key_press_timer = g_timeout_add_seconds (
                                config_get_integer ("Compile", "timer"),
                                motion_idle_cb, mc);
}

void motion_stop_timer (GuMotion* mc) {
    if (mc->key_press_timer > 0) {
        g_source_remove (mc->key_press_timer);
        mc->key_press_timer = 0;
    }
}

gboolean on_key_press_cb (GtkWidget* widget, GdkEventKey* event, void* user) {
    if (!event->is_modifier) {
        motion_stop_timer (GU_MOTION (user));
    }
    if (config_get_boolean ("Interface", "snippets") &&
        snippets_key_press_cb (gummi_get_snippets (),
                               gummi_get_active_editor (), event))
        return TRUE;
    return FALSE;
}

gboolean on_key_release_cb (GtkWidget* widget, GdkEventKey* event, void* user) {
    if (!event->is_modifier) {
        motion_start_timer (GU_MOTION (user));
    }
    if (config_get_boolean ("Interface", "snippets") &&
        snippets_key_release_cb (gummi_get_snippets (),
                                 gummi_get_active_editor (), event))
        return TRUE;
    return FALSE;
}
