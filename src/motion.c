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

#include "snippets.h"
#include "latex.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "configfile.h"
#include "editor.h"
#include "environment.h"
#include "latex.h"
#include "utils.h"
#include "gui/gui-main.h"

extern GummiGui* gui;

GuMotion* motion_init(void) {
    GuMotion* m = g_new0(GuMotion, 1);
    m->timer = 0;
    return m;
}

gboolean motion_idle_cb(void* user) {
    if (gui->previewgui->preview_on_idle)
        previewgui_update_preview(gui->previewgui);
    return FALSE;
}

void motion_start_timer(GuMotion* mc) {
    motion_stop_timer(mc);
    mc->timer = g_timeout_add_seconds(atoi(config_get_value("compile_timer")),
            motion_idle_cb, mc);
}

void motion_stop_timer(GuMotion* mc) {
    if (mc->timer > 0) {
        g_source_remove(mc->timer);
        mc->timer = 0;
    }
}
    
gboolean on_key_press_cb(GtkWidget* widget, GdkEventKey* event, void* user) {
    motion_stop_timer((GuMotion*)user);
    if (snippets_key_press_cb(gummi_get_snippets(), gummi_get_active_editor(),
                event))
        return TRUE;
    return FALSE;
}

gboolean on_key_release_cb(GtkWidget* widget, GdkEventKey* event, void* user) {
    motion_start_timer((GuMotion*)user);
    if (snippets_key_release_cb(gummi_get_snippets(), gummi_get_active_editor(),
                event))
        return TRUE;
    return FALSE;
}
