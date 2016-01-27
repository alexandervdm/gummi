/**
 * @file   motion.h
 * @brief
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

#ifndef __GUMMI_MOTION_H__
#define __GUMMI_MOTION_H__

#include <glib.h>
#include <gtk/gtk.h>

#define GU_MOTION(x) ((GuMotion*)x)
typedef struct _GuMotion GuMotion;

struct _GuMotion {
    guint key_press_timer;
    GMutex signal_mutex;
    GMutex compile_mutex;
    GThread* compile_thread;
    GCond compile_cv;
    pid_t* typesetter_pid;

    gboolean keep_running;
    gboolean pause;
    gboolean errormode;
};

GuMotion* motion_init (void);
void motion_start_compile_thread (GuMotion* m);
void motion_stop_compile_thread (GuMotion* m);
void motion_pause_compile_thread (GuMotion* m);
void motion_resume_compile_thread (GuMotion* m);
gboolean motion_do_compile (gpointer user);
void motion_force_compile (GuMotion *mc);
gpointer motion_compile_thread (gpointer data);
gboolean motion_idle_cb (gpointer user);
void motion_start_timer (GuMotion* mc);
void motion_stop_timer (GuMotion* mc);
void motion_kill_typesetter (GuMotion* m);

void motion_start_errormode (GuMotion *mc, const gchar *msg);
void motion_stop_errormode (GuMotion *mc);

gboolean on_key_press_cb (GtkWidget* widget, GdkEventKey* event, void* user);
gboolean on_key_release_cb (GtkWidget* widget, GdkEventKey* event, void* user);


#endif /* __GUMMI_MOTION_H__ */
