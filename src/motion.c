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

#include "motion.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "configfile.h"
#include "editor.h"
#include "environment.h"
#include "preview.h"
#include "utils.h"

GuMotion* motion_init(GtkBuilder* builder, GuFileInfo* fc, GuEditor* ec,
        GuPreview* pc) {
    L_F_DEBUG;
    GuMotion* m = g_new0(GuMotion, 1);

    /* initialize basis */
    m->b_finfo = fc;
    m->b_editor = ec;
    m->b_preview = pc;

    /* initialize members */
    m->statuslight =
        GTK_TOOL_BUTTON(gtk_builder_get_object(builder, "tool_statuslight"));
    const gchar* typesetter = config_get_value("typesetter");
    m->typesetter = (gchar*)g_malloc(strlen(typesetter) + 1);
    strncpy(m->typesetter, typesetter, strlen(typesetter) + 1);
    m->errorline = 0;
    m->last_errorline = 0;
    m->update = 0;
    m->timer = 0;
    m->modified_since_compile = FALSE;
    return m;
}

void motion_initial_preview(GuMotion* mc) {
    L_F_DEBUG;
    motion_update_workfile(mc);
    motion_update_pdffile(mc);
    motion_update_errortags(mc);

    /* force the preview to refresh to trash previous document */
    mc->modified_since_compile = TRUE;
    /* check for error and see if need to go into error mode */
    if (mc->errorline)
        motion_setup_preview_error_mode(mc);
    else {
        preview_set_pdffile(mc->b_preview, mc->b_finfo->pdffile);
        motion_updatepreview(mc);
    }
}

void motion_update_workfile(GuMotion* mc) {
    L_F_DEBUG;
    GtkTextIter start, end;
    gchar *text;
    FILE *fp;

    /* save selection */
    gtk_text_buffer_get_selection_bounds(
            GTK_TEXT_BUFFER(mc->b_editor->sourcebuffer), &start, &end);
    text = editor_grab_buffer(mc->b_editor);

    /* restore selection */
    gtk_text_buffer_select_range(
            GTK_TEXT_BUFFER(mc->b_editor->sourcebuffer), &start, &end);
    
    fp = fopen(mc->b_finfo->workfile, "w");
    
    if(fp == NULL) {
        slog(L_ERROR, "unable to create workfile in tmpdir\n");
        return;
    }
    fwrite(text, strlen(text), 1, fp);
    g_free(text);
    fclose(fp);
    // TODO: Maybe add editorviewer grab focus line here if necessary
}

void motion_update_pdffile(GuMotion* mc) {
    L_F_DEBUG;
    gchar* dirname = g_path_get_dirname(mc->b_finfo->workfile);
    gchar* command = g_strdup_printf("cd \"%s\";"
                                     "env openout_any=a %s "
                                     "-interaction=nonstopmode "
                                     "-file-line-error "
                                     "-halt-on-error "
                                     "-output-directory=\"%s\" \"%s\"",
                                     dirname,
                                     mc->typesetter,
                                     mc->b_finfo->tmpdir,
                                     mc->b_finfo->workfile);
    g_free(dirname);

    gtk_tool_button_set_stock_id(mc->statuslight, "gtk-refresh");
    while (gtk_events_pending()) gtk_main_iteration();
    pdata cresult = utils_popen_r(command);
    errorbuffer_set_text(cresult.data);
    mc->errorline = cresult.ret;
    mc->modified_since_compile = FALSE;

    /* find error line */
    if (cresult.ret == 1 &&
            (strstr(cresult.data, "Fatal error") ||
            (strstr(cresult.data, "No pages of output.")))) {
        gchar** result = 0;
        GError* error = NULL;
        GRegex* match_str = 0;
        GMatchInfo* match_info;
        match_str = g_regex_new(":([\\d+]+):", G_REGEX_DOTALL, 0, &error);

        if (g_regex_match(match_str, cresult.data, 0, &match_info)) {
            result = g_match_info_fetch_all(match_info);
            if (result[1])
                mc->errorline = atoi(result[1]);
            g_strfreev(result);
        }
        g_match_info_free(match_info);
        g_regex_unref(match_str);

        /* update status light */
        gtk_tool_button_set_stock_id(mc->statuslight, "gtk-no");
    } else if (strstr(cresult.data, "No pages of output.")) {
        mc->errorline = -1;
        gtk_tool_button_set_stock_id(mc->statuslight, "gtk-no");
    } else
        gtk_tool_button_set_stock_id(mc->statuslight, "gtk-yes");
    g_free(cresult.data);
    g_free(command);
}


void motion_start_updatepreview(GuMotion* mc) {
    L_F_DEBUG;
    if (0 == strcmp(config_get_value("compile_scheme"), "on_idle")) {
        mc->shandlers[0] = g_signal_connect(mc->b_editor->sourceview,
                                            "key-press-event",
                                            G_CALLBACK(on_key_press_cb),
                                            (void*)mc);
        mc->shandlers[1] = g_signal_connect(mc->b_editor->sourceview,
                                            "key-release-event",
                                            G_CALLBACK(on_key_release_cb),
                                            (void*)mc);
        motion_start_timer(mc);
    } else  {
        mc->update = g_timeout_add_seconds(
                atoi(config_get_value("compile_timer")),
                motion_updatepreview, (void*)mc);
    }
}

void motion_stop_updatepreview(GuMotion* mc) {
    L_F_DEBUG;
    if (0 == strcmp(config_get_value("compile_scheme"), "on_idle")) {
        g_signal_handler_disconnect(mc->b_editor->sourceview, mc->shandlers[0]);
        g_signal_handler_disconnect(mc->b_editor->sourceview, mc->shandlers[1]);
        motion_stop_timer(mc);
    } else if (mc->update > 0) {
        g_source_remove(mc->update);
        mc->update = 0;
    }
}

void motion_update_auxfile(GuMotion* mc) {
    L_F_DEBUG;
    gchar* dirname = g_path_get_dirname(mc->b_finfo->workfile);
    gchar* command = g_strdup_printf("cd \"%s\";"
                                     "env openout_any=a %s "
                                     "--draftmode "
                                     "-interaction=nonstopmode "
                                     "--output-directory=\"%s\" \"%s\"",
                                     dirname,
                                     mc->typesetter,
                                     mc->b_finfo->tmpdir,
                                     mc->b_finfo->workfile);
    g_free(dirname);
    pdata res = utils_popen_r(command);
    g_free(res.data);
    g_free(command);
}

void motion_export_pdffile(GuMotion* mc, const gchar* path) {
    L_F_DEBUG;
    gchar* savepath;
    gint ret = 0;

    if (0 != strcmp(path + strlen(path) -4, ".pdf"))
        savepath = g_strdup_printf("%s.pdf", path);
    else
        savepath = g_strdup(path);
    if (utils_path_exists(savepath)) {
        ret = utils_yes_no_dialog(_("The file already exists. Overwrite?"));
        if (GTK_RESPONSE_YES != ret) {
            g_free(savepath);
            return;
        }
    }
    utils_copy_file(mc->b_finfo->pdffile, savepath);
    g_free(savepath);
}

void motion_update_errortags(GuMotion* mc) {
    L_F_DEBUG;
    if (mc->errorline > 0)
        editor_apply_errortags(mc->b_editor, mc->errorline);
    if (mc->last_errorline && !mc->errorline)
        editor_apply_errortags(mc->b_editor, 0);
    mc->last_errorline = mc->errorline;
}

gboolean motion_updatepreview(void* user) {
    L_F_DEBUG;
    GuMotion* mc = (GuMotion*)user;
    if (mc->modified_since_compile) {
      motion_update_workfile(mc);
      motion_update_pdffile(mc);
      motion_update_errortags(mc);
      preview_refresh(mc->b_preview);
    }
    return 0 != strcmp(config_get_value("compile_scheme"), "on_idle");
}

void motion_setup_preview_error_mode(GuMotion* mc) {
    L_F_DEBUG;

    GtkEventBox* eventbox = GTK_EVENT_BOX(gtk_event_box_new());
    gtk_widget_set_events(GTK_WIDGET(eventbox), GDK_BUTTON_PRESS_MASK);
    g_signal_connect(eventbox, "button-press-event",
            G_CALLBACK(on_error_button_press), mc);
    char* message = g_strdup_printf(_("PDF-Preview could not initialize.\n\n"
            "It appears your LaTeX document contains errors or\n"
            "the program `%s' was not installed.\n"
            "Additional information is available on the Error Output tab.\n"
            "Please correct the listed errors and click this area\n"
            "to reload the preview panel."), mc->typesetter);
    GtkLabel* label = GTK_LABEL(gtk_label_new(message));
    g_free(message);
    gtk_label_set_justify(label, GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(eventbox), GTK_WIDGET(label));
    gtk_container_remove(GTK_CONTAINER(mc->b_preview->preview_viewport),
            GTK_WIDGET(mc->b_preview->drawarea));
    gtk_container_add(GTK_CONTAINER(mc->b_preview->preview_viewport),
            GTK_WIDGET(eventbox));
    gtk_widget_show_all(GTK_WIDGET(mc->b_preview->preview_viewport));
}

void on_error_button_press(GtkWidget* widget, GdkEventButton* event, void* m) {
    L_F_DEBUG;

    GuMotion* mc = (GuMotion*)m;
    motion_update_workfile(mc);
    motion_update_pdffile(mc);
    motion_update_errortags(mc);

    if (!mc->errorline) {
        gtk_container_remove(GTK_CONTAINER(mc->b_preview->preview_viewport),
                widget);
        gtk_container_add(GTK_CONTAINER(mc->b_preview->preview_viewport),
                GTK_WIDGET(mc->b_preview->drawarea));
        if (config_get_value("compile_status") &&
            0 == strcmp(config_get_value("compile_scheme"), "on_idle")) {
            preview_set_pdffile(mc->b_preview, mc->b_finfo->pdffile);
            motion_updatepreview(mc);
        }
    }
}

void motion_start_timer(GuMotion* mc) {
    L_F_DEBUG;
    motion_stop_timer(mc);
    mc->timer = g_timeout_add_seconds(
            atoi(config_get_value("compile_timer")),
            motion_updatepreview, (void*)mc);
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
    return FALSE;
}

gboolean on_key_release_cb(GtkWidget* widget, GdkEventKey* event, void* user) {
    L_F_DEBUG;
    GuMotion* mc = (GuMotion*)user;
    if (mc->modified_since_compile)
        motion_start_timer(mc);
    return FALSE;
}
