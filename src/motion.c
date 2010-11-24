/**
 * @file   motion.c
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

#include "latex.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "configfile.h"
#include "editor.h"
#include "environment.h"
#include "latex.h"
#include "utils.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuMotion* motion_init(GuEditor* ec) {
    L_F_DEBUG;
    GuMotion* m = g_new0(GuMotion, 1);
    m->timer = 0;
    g_signal_connect(ec->sourceview, "key-press-event",
            G_CALLBACK(on_key_press_cb), m);
    g_signal_connect(ec->sourceview, "key-release-event",
            G_CALLBACK(on_key_release_cb), m);
    return m;
}

gboolean motion_idle_cb(void* user) {
    if (gui->previewgui->preview_on_idle)
        previewgui_update_preview(gui->previewgui);
    return FALSE;
}

void motion_start_timer(GuMotion* mc) {
    L_F_DEBUG;
    motion_stop_timer(mc);
    mc->timer = g_timeout_add_seconds(atoi(config_get_value("compile_timer")),
            motion_idle_cb, mc);
}

void motion_stop_timer(GuMotion* mc) {
    L_F_DEBUG;
    if (mc->timer > 0) {
        g_source_remove(mc->timer);
        mc->timer = 0;
    }
}
    
gboolean on_key_press_cb(GtkWidget* widget, GdkEventKey* event, void* user) {
    L_F_DEBUG;
    motion_stop_timer((GuMotion*)user);
    //snippets_key_press_cb(event);
    return FALSE;
}

gboolean on_key_release_cb(GtkWidget* widget, GdkEventKey* event, void* user) {
    L_F_DEBUG;
    motion_start_timer((GuMotion*)user);
    return FALSE;
}
