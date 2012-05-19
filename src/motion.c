/**
 * @file   motion.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#ifndef WIN32
    #include <sys/types.h>
    #include <sys/wait.h>
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

/* set up uri using appropriate formatting for OS
   http://en.wikipedia.org/wiki/File_URI_scheme#Linux */
#ifdef WIN32
    const gchar *urifrmt = "file:///";
#else
    const gchar *urifrmt = "file://";
#endif


extern GummiGui* gui;
extern Gummi* gummi;

/* Typesetter pid */
pid_t typesetter_pid = 0;

GuMotion* motion_init (void) {
    GuMotion* m = g_new0 (GuMotion, 1);

    m->key_press_timer = 0;
    m->signal_mutex = g_mutex_new ();
    m->compile_mutex = g_mutex_new ();
    m->compile_cv = g_cond_new ();
    m->keep_running = TRUE;
    m->typesetter_pid = &typesetter_pid;

    return m;
}

void motion_start_compile_thread (GuMotion* m) {
    GError* err = NULL;

    m->keep_running = TRUE;
    m->compile_thread = g_thread_create (motion_compile_thread, m, TRUE, &err);
    if (!m->compile_thread) {
        slog (L_G_FATAL, "Can not create new thread: %s\n", err->message);
        g_error_free(err);
    }
}

void motion_stop_compile_thread (GuMotion* m) {
    m->keep_running = FALSE;
    motion_do_compile(m);
    g_thread_join(m->compile_thread);
}

void motion_kill_typesetter (GuMotion* m) {
    if (*m->typesetter_pid) {
        gchar* command = NULL;
        /* Kill children spawned by typesetter command/script, don't know
         * how to do this programatically yet(glib doesn't not provides any
         * function for killing a process), so use pkill for now. For
         * win32 there's currently nothing we can do about it. */
#ifndef WIN32
        command = g_strdup_printf("pkill -15 -P %d", *m->typesetter_pid);
        system(command);
        g_free(command);

        /* Make sure typesetter command is terminated */
        kill(*m->typesetter_pid, 15);
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

    if (!g_mutex_trylock (mc->signal_mutex)) goto ret;
    g_cond_signal (mc->compile_cv);
    g_mutex_unlock (mc->signal_mutex);

ret:
    return (STR_EQU (config_get_value ("compile_scheme"), "real_time"));
}

gpointer motion_compile_thread (gpointer data) {
    L_F_DEBUG;
    GuMotion* mc = GU_MOTION (data);
    GuEditor* editor = NULL;
    GuLatex* latex = NULL;
    GuPreviewGui* pc = NULL;
    GtkWidget* focus = NULL;
    gboolean precompile_ok = FALSE;
    gboolean compile_status = FALSE;
    gchar *editortext;

    latex = gummi_get_latex ();
    pc = gui->previewgui;
    
    while (TRUE) {
        if (!g_mutex_trylock (mc->compile_mutex)) continue;
        slog (L_DEBUG, "Compile thread sleeping...\n");
        g_cond_wait (mc->compile_cv, mc->compile_mutex);
        slog (L_DEBUG, "Compile thread awoke.\n");

        if (!(editor = gummi_get_active_editor ())) {
            g_mutex_unlock (mc->compile_mutex);
            continue;
        }
        if (!mc->keep_running) {
            g_mutex_unlock (mc->compile_mutex);
            g_thread_exit (NULL);
        }

        gdk_threads_enter ();
        focus = gtk_window_get_focus (gui->mainwindow);
        editortext = latex_update_workfile (latex, editor);
        
        precompile_ok = latex_precompile_check (editortext);
        g_free (editortext);

        gtk_widget_grab_focus (focus);
        gdk_threads_leave();

        if (!precompile_ok) {
            g_mutex_unlock (mc->compile_mutex);
            gdk_threads_enter();
            motion_start_errormode (mc, "document_error");
            gdk_threads_leave();
            continue;
        }
        
        compile_status = latex_update_pdffile (latex, editor);
        *mc->typesetter_pid = 0;
        g_mutex_unlock (mc->compile_mutex);

        if (!mc->keep_running)
            g_thread_exit (NULL);

        gdk_threads_enter ();
        previewgui_update_statuslight(compile_status? "gtk-yes": "gtk-no");

        /* Make sure the editor still exists after compile */
        if (editor == gummi_get_active_editor()) {
            editor_apply_errortags (editor, latex->errorlines);
            gui_buildlog_set_text (latex->compilelog);

            if (latex->errorlines[0]) {
                motion_start_errormode  (mc, "compile_error");
            } else {
                if (!pc->uri) {
                    
                    char* uri = g_strconcat (urifrmt, editor->pdffile, NULL);
                    previewgui_set_pdffile (pc, uri);
                    g_free(uri);
                } else {
                    previewgui_refresh (gui->previewgui,
                            editor->sync_to_last_edit ?
                            &(editor->last_edit) : NULL, editor->workfile);
                }
                if (mc->errormode) motion_stop_errormode (mc);
            }
        }
        gdk_threads_leave ();
    }
}

void motion_force_compile (GuMotion *mc) {
    /* sort-of signal to force a compile run after certain actions that
     * don't trigger the regular editor content change signals */
    gummi->latex->modified_since_compile = TRUE;
    motion_do_compile (mc);
}

void motion_start_errormode (GuMotion *mc, const gchar *msg) {
    
    if (mc->errormode) {
        infoscreengui_set_message (gui->infoscreengui, msg);
    return;
    }

    previewgui_save_position (gui->previewgui);
    
    infoscreengui_enable (gui->infoscreengui, msg);
    mc->errormode = TRUE;
}

void motion_stop_errormode (GuMotion *mc) {
    
    if (!mc->errormode) return;

    previewgui_restore_position (gui->previewgui);
    
    infoscreengui_disable (gui->infoscreengui);
    mc->errormode = FALSE;
}

gboolean motion_idle_cb (gpointer user) {
    if (gui->previewgui->preview_on_idle)
        motion_do_compile (GU_MOTION (user));
    return FALSE;
}

void motion_start_timer (GuMotion* mc) {
    motion_stop_timer (mc);
    mc->key_press_timer = g_timeout_add_seconds (atoi (
                config_get_value ("compile_timer")), motion_idle_cb, mc);
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
    if (config_get_value("snippets") && 
        snippets_key_press_cb (gummi_get_snippets (),
                               gummi_get_active_editor (), event))
        return TRUE;
    return FALSE;
}

gboolean on_key_release_cb (GtkWidget* widget, GdkEventKey* event, void* user) {
    if (!event->is_modifier) {
        motion_start_timer (GU_MOTION (user));
    }
    if (config_get_value("snippets") && 
        snippets_key_release_cb (gummi_get_snippets (),
                                 gummi_get_active_editor (), event))
        return TRUE;
    return FALSE;
}
