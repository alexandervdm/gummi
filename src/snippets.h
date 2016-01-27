/**
 * @file    snippets.h
 * @brief   handle configuration file
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

#ifndef __GUMMI_SNIPPETS_H__
#define __GUMMI_SNIPPETS_H__

#include <stdio.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "editor.h"
#include "utils.h"

/* Storing placeholders */
#define GU_SNIPPET_EXPAND_INFO(x) ((GuSnippetExpandInfo*)x)
typedef struct _GuSnippetExpandInfo GuSnippetExpandInfo;

struct _GuSnippetExpandInfo {
    glong group_number;
    GtkTextMark* right_mark;
    GtkTextMark* left_mark;
    gint start;
    gint len;
    gchar* text;
};


/* Storing single snippet info */
#define GU_SNIPPET_INFO(x) ((GuSnippetInfo*)x)
typedef struct _GuSnippetInfo GuSnippetInfo;

struct _GuSnippetInfo {
    gchar* snippet;
    gchar* expanded;
    gint start_offset;
    gint offset;
    GList* current;
    GList* einfo;        /* List of GuSnippetExpandInfo sorted by start pos */
    GList* einfo_unique; /* List of unique group einfo */
    GList* einfo_sorted; /* List of GuSnippetExpandInfo sorted by #group */

    /* Constants */
    gchar* sel_text;
    GtkTextMark sel_start;
};


#define GU_SNIPPETS(x) ((GuSnippets*)x)
typedef struct _GuSnippets GuSnippets;

struct _GuSnippets {
    gchar* filename;
    slist* head;
    GuSnippetInfo* info;
    GtkAccelGroup* accel_group;
    GList* stackframe;
    GList* closure_data; /* data: Tuple2 (key, closure) */
};

GuSnippets* snippets_init (const gchar* filename);
void snippets_set_default (GuSnippets* sc);
void snippets_load (GuSnippets* sc);
void snippets_save (GuSnippets* sc);
void snippets_clean_up (GuSnippets* sc);
gchar* snippets_get_value (GuSnippets* sc, const gchar* term);
void snippets_set_accelerator (GuSnippets* sc, gchar* config);
void snippets_activate (GuSnippets* sc, GuEditor* ec, gchar* key);
void snippets_deactivate (GuSnippets* sc, GuEditor* ec);
gboolean snippets_key_press_cb (GuSnippets* sc, GuEditor* ec, GdkEventKey* ev);
gboolean snippets_key_release_cb(GuSnippets* sc, GuEditor* ec, GdkEventKey* ev);
GuSnippetInfo* snippets_parse (char* snippet);
void snippets_accel_cb (GtkAccelGroup* accel_group, GObject* obj,
        guint keyval, GdkModifierType mods, Tuple2* udata);
void snippets_accel_connect (GuSnippets* sc, guint keyval, GdkModifierType mod,
        GClosure* closure);
void snippets_accel_disconnect (GuSnippets* sc, const gchar* key);

GuSnippetInfo* snippet_info_new (gchar* snippet);
void snippet_info_free (GuSnippetInfo* info, GuEditor* ec);
void snippet_info_append_holder (GuSnippetInfo* info, gint group, gint start,
        gint len, gchar* text);
void snippet_info_create_marks (GuSnippetInfo* info, GuEditor* ec);
void snippet_info_remove_marks (GuSnippetInfo* info, GuEditor* ec);
void snippet_info_initial_expand (GuSnippetInfo* info, GuEditor* ec);
gboolean snippet_info_goto_next_placeholder (GuSnippetInfo* info, GuEditor* ec);
gboolean snippet_info_goto_prev_placeholder (GuSnippetInfo* info, GuEditor* ec);
void snippet_info_sync_group (GuSnippetInfo* info, GuEditor* ec);
gint snippet_info_num_cmp (gconstpointer a, gconstpointer b);
gint snippet_info_pos_cmp (gconstpointer a, gconstpointer b);

#endif /* __GUMMI_SNIPPETS__ */
