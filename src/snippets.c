/**
 * @file    snippets.c
 * @brief   handle snppets
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <gdk/gdkkeysyms.h>
#include <glib.h>

#include "editor.h"
#include "gui/gui-editortabs.h"
#include "utils.h"

GuSnippets* snippets_init(const gchar* filename) {
    GuSnippets* s = g_new0(GuSnippets, 1);
    gchar* dirname = g_path_get_dirname(filename);
    g_mkdir_with_parents(dirname,
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    g_free(dirname);

    slog(L_INFO, "snippets : %s\n", filename);

    s->filename = g_strdup(filename);
    s->accel_group = gtk_accel_group_new();
    snippets_load(s);
    return s;
}

void snippets_set_default(GuSnippets* sc) {
    FILE* fh = 0;
    if (!(fh = fopen(sc->filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    fclose(fh);
}

void snippets_load(GuSnippets* sc) {
    FILE* fh = 0;
    gchar buf[BUFSIZ];
    gchar* rot = NULL;
    gchar* seg = NULL;
    slist* current = NULL;
    slist* prev = NULL;

    if (sc->head)
        snippets_clean_up(sc);

    if (!(fh = fopen(sc->filename, "r"))) {
        slog(L_ERROR, "can't find snippets file, reseting to default\n");
        return snippets_load(sc);
    }

    current = sc->head = prev = g_new0(slist, 1);

    while (fgets(buf, BUFSIZ, fh)) {
        buf[strlen(buf) -1] = 0; /* remove trailing '\n' */
        if (buf[0] != '\t') {
            if ('#' == buf[0]) {
                current->first = g_strdup(buf);
            } else {
                seg = strstr(buf, " ") + 1;
                current->first = g_strdup((seg == buf)? "Invalid": seg);
                snippets_set_accelerator(sc, current->first);
            }
        } else {
            if (!prev->second) {
                prev->second = g_strdup(buf + 1);
                continue;
            }
            rot = g_strdup(prev->second);
            g_free(prev->second);
            prev->second = g_strconcat(rot, "\n", buf + 1, NULL);
            g_free(rot);
            g_free(current);
            current = prev;
        }
        prev = current;
        current->next = g_new0(slist, 1);
        current = current->next;
    }
    g_free(current);
    prev->next = NULL;
    fclose(fh);
}

void snippets_save(GuSnippets* sc) {
    FILE* fh = 0;
    slist* current = sc->head;
    gint i = 0, count = 0, len = 0;
    gchar* buf = 0;

    if (!(fh = fopen(sc->filename, "w")))
        slog(L_FATAL, "can't open snippets file for writing... abort\n");

    while (current) {
        /* skip comments */
        if ('#' == current->first[0]) {
            fputs(current->first, fh);
            fputs("\n", fh);
            current = current->next;
            continue;
        }
        fputs("snippet ", fh);
        fputs(current->first, fh);
        fputs("\n\t", fh);

        len = strlen(current->second) + 1;
        buf = (gchar*)g_malloc(len * 2);
        memset(buf, 0, len * 2);
        /* replace '\n' with '\n\t' for options with multi-line content */
        for (i = 0; i < len; ++i) {
            if (count + 2 == len * 2) break;
            buf[count++] = current->second[i];
            if (i != len -2 && '\n' == current->second[i])
                buf[count++] = '\t';
        }
        fputs(buf, fh);
        fputs("\n", fh);
        current = current->next;
        count = 0;
        g_free(buf);
    }
    fclose(fh);
}

void snippets_clean_up(GuSnippets* sc) {
    slist* prev = sc->head;
    slist* current;
    while (prev) {
        current = prev->next;
        g_free(prev);
        prev = current;
    }
    sc->head = NULL;
}

gchar* snippets_get_value(GuSnippets* sc, const gchar* term) {
    gchar* key = g_strdup_printf("%s,", term);
    slist* index = slist_find_index_of(sc->head, key, TRUE, FALSE);
    g_free(key);
    return (index)? index->second: NULL;
}

void snippets_set_accelerator(GuSnippets* sc, gchar* config) {
    /* config has the form: Key,Accel_key,Name */
    GClosure* closure = NULL;
    GdkModifierType mod;
    guint keyval = 0;
    gchar** configs = g_strsplit(config, ",", 0);
    Tuple2* data = g_new0(Tuple2, 1);
    Tuple2* closure_data = g_new0(Tuple2, 1);

    /* Return if config does not contains accelerator */
    if (strlen(configs[1]) == 0) {
        g_strfreev(configs);
        return;
    }

    data->first = (gpointer)sc;
    data->second = (gpointer)g_strdup(configs[0]);


    closure = g_cclosure_new(G_CALLBACK(snippets_accel_cb), data, NULL);
    closure_data->first = (gpointer)data->second;
    closure_data->second = (gpointer)closure;

    sc->closure_data = g_list_append(sc->closure_data, closure_data);
    gtk_accelerator_parse(configs[1], &keyval, &mod);

    /* Return without connect if accel is not valid */
    if (!gtk_accelerator_valid(keyval, mod)) return;

    snippets_accel_connect(sc->accel_group, keyval, mod, closure);
    g_strfreev(configs);
}

void snippets_activate(GuSnippets* sc, GuEditor* ec, gchar* key) {
    gchar* snippet = NULL;
    GtkTextIter start, end;
    snippet = snippets_get_value(sc, key);
    g_return_if_fail(snippet != NULL);

    editor_get_current_iter(ec, &start);
    sc->info = snippets_parse(snippet);
    sc->info->start_offset = gtk_text_iter_get_offset(&start);

    /* Prepare constants */
    gtk_text_buffer_get_selection_bounds(ec_sourcebuffer, &start, &end);
    sc->info->sel_text = gtk_text_iter_get_text(&start, &end);
    GSList* marks = gtk_text_iter_get_marks(&start);
    sc->info->sel_start = *GTK_TEXT_MARK(marks->data);
    g_slist_free(marks);

    gtk_text_buffer_insert(ec_sourcebuffer, &start, sc->info->expanded, -1);
    snippet_info_create_marks(sc->info, ec);
    snippet_info_initial_expand(sc->info, ec);
    snippet_info_replace_constants(sc->info, ec);
    gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);

    if (!snippet_info_goto_next_placeholder(sc->info, ec))
        snippets_deactivate(sc, ec);
    sc->activated = TRUE;
}

void snippets_deactivate(GuSnippets* sc, GuEditor* ec) {
    sc->activated = FALSE;
    snippet_info_free(sc->info, ec);
}

gboolean snippets_key_press_cb(GuSnippets* sc, GuEditor* ec, GdkEventKey* ev) {
    GtkTextIter current, start;

    if (ev->keyval != GDK_KEY_Tab && !sc->activated)
       return FALSE;

    if (sc->activated) {
        if (ev->keyval == GDK_KEY_Tab) {
            if (!snippet_info_goto_next_placeholder(sc->info, ec))
                snippets_deactivate(sc, ec);
            return TRUE;
        } else if (ev->keyval == GDK_KEY_ISO_Left_Tab
                && ev->state & GDK_SHIFT_MASK) {
            snippet_info_goto_prev_placeholder(sc->info, ec);
            return TRUE;
        }
        /* Deactivate snippet if the current insert range is not within the
         * snippet */
        editor_get_current_iter(ec, &current);
        gint offset = gtk_text_iter_get_offset(&current);
        GList* last = g_list_last(sc->info->einfo);
        if (last) {
            gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &current,
                    GU_SNIPPET_EXPAND_INFO(last->data)->left_mark);
            gint bound_end = gtk_text_iter_get_offset(&current);
            if (offset < sc->info->start_offset || offset > bound_end)
                snippets_deactivate(sc, ec);
        }
    } else {
        gchar* key = NULL;
        editor_get_current_iter(ec, &current);
        if (!gtk_text_iter_ends_word(&current)) return FALSE;
        start = current;
        gtk_text_iter_backward_word_start(&start);
        key = gtk_text_iter_get_text(&start, &current);
        if (snippets_get_value(sc, key)) {
            gtk_text_buffer_delete(ec_sourcebuffer, &start, &current);
            snippets_activate(sc, ec, key);
        }
        g_free(key);
        return sc->activated;
    }
    return FALSE;
}

gboolean snippets_key_release_cb(GuSnippets* sc, GuEditor* ec, GdkEventKey* e) {
    if (e->keyval != GDK_KEY_Tab && !sc->activated)
       return FALSE;

    if (sc->activated) {
        snippet_info_sync_group(sc->info, ec);
        return FALSE;
    }
    return FALSE;
}

GuSnippetInfo* snippets_parse(char* snippet) {
    gint i, start, end;
    GError* err = NULL;
    gchar** result = NULL;
    GRegex* regex = NULL;
    GMatchInfo* match_info = NULL;
    const gchar* holders[] = { "\\$([0-9]+)", "\\${([0-9]*):?([^}]*)}" };

    GuSnippetInfo* info = snippet_info_new(snippet);

    for (i = 0; i < 2; ++i) {
        if (!(regex = g_regex_new(holders[i], G_REGEX_DOTALL, 0, &err))) {
            slog(L_ERROR, "g_regex_new(): %s\n", err->message);
            g_error_free(err);
            return info;
        }
        g_regex_match(regex, snippet, 0, &match_info);
        while (g_match_info_matches(match_info)) {
            result = g_match_info_fetch_all(match_info);
            g_match_info_fetch_pos(match_info, 0, &start, &end);
            snippet_info_append_holder(info, atoi(result[1]), start,
                    end -start, result[2]);
            g_match_info_next(match_info, NULL);
            g_strfreev(result);
        }
        g_regex_unref(regex);
        g_match_info_free(match_info);
    }

    info->einfo = g_list_sort(info->einfo, snippet_info_pos_cmp);
    info->einfo_sorted = g_list_copy(info->einfo);
    info->einfo_sorted = g_list_sort(info->einfo_sorted, snippet_info_num_cmp);
    return info;
}

void snippets_accel_cb(GtkAccelGroup* accel_group, GObject* obj,
        guint keyval, GdkModifierType mods, Tuple2* udata) {
    GuSnippets* sc = (GuSnippets*)udata->first;
    gchar* key = (gchar*)udata->second;
    /* XXX: Don't know how to avoid using get_active_editor() here. Since
     * gtk_accel_group must be connect when load, we can not specify the
     * editor in user_data, because snippets should only have effect on
     * active tab */
    snippets_activate(sc, get_active_editor(), key);
}

void snippets_accel_connect(GtkAccelGroup* accel_group, guint keyval,
        GdkModifierType mod, GClosure* closure) {
    gtk_accel_group_connect(accel_group, keyval, mod, GTK_ACCEL_VISIBLE,
            closure);
}

void snippets_accel_disconnect(GtkAccelGroup* accel_group, GClosure* closure) {
    gtk_accel_group_disconnect(accel_group, closure);
}

GuSnippetInfo* snippet_info_new(gchar* snippet) {
    GuSnippetInfo* info = g_new0(GuSnippetInfo, 1);
    info->snippet = g_strdup(snippet);
    info->expanded = g_strdup(snippet);
    info->einfo = NULL;
    info->einfo_sorted = NULL;
    return info;
}

void snippet_info_free(GuSnippetInfo* info, GuEditor* ec) {
    snippet_info_remove_marks(info, ec);
    GList* current = g_list_first(info->einfo);
    while (current) {
        g_free(GU_SNIPPET_EXPAND_INFO(current->data)->text);
        current = g_list_next(current);
    }
    g_list_free(info->einfo);
    g_list_free(info->einfo_sorted);
}

gboolean snippet_info_goto_next_placeholder(GuSnippetInfo* info, GuEditor* ec) {
    GuSnippetExpandInfo* einfo = NULL;
    GtkTextIter start, end;
    gboolean success = TRUE;

    if (!info->current) {
        if (info->einfo_unique
            && GU_SNIPPET_EXPAND_INFO(info->einfo_unique->data)->group_number
            == 0)
            info->current = g_list_next(info->einfo_unique);
        else
            info->current = info->einfo_unique;
    } else
        info->current = g_list_next(info->current);

    if (!info->current) {
        info->current = g_list_first(info->einfo_sorted);
        if (!info->current
            || GU_SNIPPET_EXPAND_INFO(info->current->data)->group_number != 0)
            return FALSE;
        success = FALSE;
    }
    einfo = GU_SNIPPET_EXPAND_INFO(info->current->data);
    gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &start, einfo->left_mark);
    gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &end, einfo->right_mark);
    gtk_text_buffer_place_cursor(ec_sourcebuffer, &start);
    gtk_text_buffer_select_range(ec_sourcebuffer, &start, &end);
    return success;
}

void snippet_info_goto_prev_placeholder(GuSnippetInfo* info, GuEditor* ec) {
    GuSnippetExpandInfo* einfo = NULL;
    GtkTextIter start, end;
    if (!info->current)
        return;
    else
        info->current = g_list_previous(info->current);
    einfo = GU_SNIPPET_EXPAND_INFO(info->current->data);
    gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &start, einfo->left_mark);
    gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &end, einfo->right_mark);
    gtk_text_buffer_place_cursor(ec_sourcebuffer, &start);
    gtk_text_buffer_select_range(ec_sourcebuffer, &start, &end);
}

void snippet_info_append_holder(GuSnippetInfo* info, gint group, gint start,
        gint len, gchar* text) {
    GuSnippetExpandInfo* einfo = g_new0(GuSnippetExpandInfo, 1);
    einfo->group_number = group;
    einfo->start = start;
    einfo->len = len;
    einfo->text = g_strdup(text? text: "");
    info->einfo = g_list_append(info->einfo, einfo);
}

void snippet_info_create_marks(GuSnippetInfo* info, GuEditor* ec) {
    GList* current = g_list_first(info->einfo);
    GtkTextIter start, end;

    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        gtk_text_buffer_get_iter_at_offset(ec_sourcebuffer, &start,
                info->start_offset + einfo->start);
        gtk_text_buffer_get_iter_at_offset(ec_sourcebuffer, &end,
                info->start_offset + einfo->start + einfo->len);
        einfo->left_mark = gtk_text_mark_new(NULL, TRUE);
        einfo->right_mark = gtk_text_mark_new(NULL, FALSE);
        gtk_text_buffer_add_mark(ec_sourcebuffer, einfo->left_mark, &start);
        gtk_text_buffer_add_mark(ec_sourcebuffer, einfo->right_mark, &end);
        current = g_list_next(current);
    }
}

void snippet_info_remove_marks(GuSnippetInfo* info, GuEditor* ec) {
    GList* current = g_list_first(info->einfo);

    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        gtk_text_buffer_delete_mark(ec_sourcebuffer, einfo->left_mark);
        gtk_text_buffer_delete_mark(ec_sourcebuffer, einfo->right_mark);
        current = g_list_next(current);
    }
}

void snippet_info_initial_expand(GuSnippetInfo* info, GuEditor* ec) {
    GHashTable* map = g_hash_table_new(NULL, NULL);
    GList* current = g_list_first(info->einfo);
    GuSnippetExpandInfo* value = NULL;
    GtkTextIter start, end;
    gint key = 0;

    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        if (!g_hash_table_lookup_extended(map, ((gpointer)einfo->group_number),
                    (gpointer)&key, (gpointer)&value)) {
            g_hash_table_insert(map, (gpointer)einfo->group_number, einfo);
            info->einfo_unique = g_list_append(info->einfo_unique, einfo);
        }
        current = g_list_next(current);
    }
    info->einfo_unique = g_list_sort(info->einfo_unique, snippet_info_num_cmp);

    current = g_list_first(info->einfo);
    info->offset = 0;
    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        g_hash_table_lookup_extended(map, (gpointer)einfo->group_number,
                (gpointer)&key, (gpointer)&value);
        gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &start,
                einfo->left_mark);
        gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &end,
                einfo->right_mark);
        gtk_text_buffer_delete(ec_sourcebuffer, &start, &end);
        gtk_text_buffer_insert(ec_sourcebuffer, &start, einfo->text, -1);
        current = g_list_next(current);
    }
    g_hash_table_destroy(map);
}

void snippet_info_replace_constants(GuSnippetInfo* info, GuEditor* ec) {
    GList* current = g_list_first(info->einfo);
    gchar* text = NULL;
    GtkTextIter start, end;

    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &start,
                einfo->left_mark);
        gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &end,
                einfo->right_mark);
        text = gtk_text_iter_get_text(&start, &end);
        if (strcmp(text, "$SELECTED_TEXT") == 0) {
            GtkTextIter ms, me;
            gtk_text_buffer_delete(ec_sourcebuffer, &start, &end);
            gtk_text_buffer_insert(ec_sourcebuffer, &start, info->sel_text, -1);
            gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &ms,
                    &info->sel_start);
            me = ms;
            gtk_text_iter_forward_chars(&me, strlen(info->sel_text));
            gtk_text_buffer_delete(ec_sourcebuffer, &ms, &me);
        } else if (strcmp(text, "$FILENAME") == 0) {
            gtk_text_buffer_delete(ec_sourcebuffer, &start, &end);
            gtk_text_buffer_insert(ec_sourcebuffer, &start,
                    ec->filename? ec->filename: "", -1);
        } else if (strcmp(text, "$BASENAME") == 0) {
            gchar* basename = g_path_get_basename(ec->filename?ec->filename:"");
            gtk_text_buffer_delete(ec_sourcebuffer, &start, &end);
            gtk_text_buffer_insert(ec_sourcebuffer, &start, basename, -1);
            g_free(basename);
        }
        g_free(text);
        current = g_list_next(current);
    }
}

void snippet_info_sync_group(GuSnippetInfo* info, GuEditor* ec) {
    if (!info->current) return;
    GuSnippetExpandInfo* active = GU_SNIPPET_EXPAND_INFO(info->current->data);
    GList* current = g_list_first(info->einfo);
    gchar* text = NULL;
    GtkTextIter start, end;

    gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &start,
            active->left_mark);
    gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &end,
            active->right_mark);
    text = gtk_text_buffer_get_text(ec_sourcebuffer, &start, &end, TRUE);

    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        if (einfo != active && einfo->group_number == active->group_number) {
            gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &start,
                    einfo->left_mark);
            gtk_text_buffer_get_iter_at_mark(ec_sourcebuffer, &end,
                    einfo->right_mark);
            gtk_text_buffer_delete(ec_sourcebuffer, &start, &end);
            gtk_text_buffer_insert(ec_sourcebuffer, &start, text, -1);
        }
        current = g_list_next(current);
    }
    g_free(text);
}

gint snippet_info_num_cmp(gconstpointer a, gconstpointer b) {
    return ((GU_SNIPPET_EXPAND_INFO(a)->group_number <
                GU_SNIPPET_EXPAND_INFO(b)->group_number)? -1: 1);
}

gint snippet_info_pos_cmp(gconstpointer a, gconstpointer b) {
    return ((GU_SNIPPET_EXPAND_INFO(a)->start <
                GU_SNIPPET_EXPAND_INFO(b)->start)? -1: 1);
}
