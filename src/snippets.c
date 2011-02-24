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
#include "utils.h"

GuSnippets* snippets_init(const gchar* filename) {
    GuSnippets* s = g_new0(GuSnippets, 1);
    gchar* dirname = g_path_get_dirname(filename);

    g_mkdir_with_parents(dirname,
            S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    g_free(dirname);

    slog(L_INFO, "snippets : %s\n", filename);

    s->filename = g_strdup(filename);
    snippets_load(s);
    return s;
}

void snippets_set_default(GuSnippets* sc) {
    FILE* fh = 0;
    if (!(fh = fopen(sc->filename, "w")))
        slog(L_FATAL, "can't open config for writing... abort\n");

    //fwrite(snippets_str, strlen(snippets_str), 1, fh);
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
        snippets_set_default(sc);
        return snippets_load(sc);
    }

    current = sc->head = prev = g_new0(slist, 1);

    while (fgets(buf, BUFSIZ, fh)) {
        buf[strlen(buf) -1] = 0; /* remove trailing '\n' */
        if (buf[0] != '\t') {
            if ('#' == buf[0]) {
                current->first = g_strdup(buf);
            } else {
                seg = strtok(buf, " ");
                seg = strtok(NULL, " ");
                current->first = g_strdup((seg == buf)? "Invalid": seg);
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
    slist* index = slist_find_index_of(sc->head, term, FALSE);
    return (index)? index->second: NULL;
}

gboolean snippets_key_press_cb(GuSnippets* sc, GuEditor* ec,
        GdkEventKey* event) {
    static GtkTextIter current, start;
    static GuSnippetInfo* info = NULL;

    if (event->keyval != GDK_KEY_Tab && !ec->snippet_editing)
       return FALSE;

    if (ec->snippet_editing) {
        if (event->keyval == GDK_KEY_Tab)
            snippet_info_jump_to_next_placeholder(info, ec);
        else if (event->keyval == GDK_KEY_ISO_Left_Tab
                && event->state & GDK_SHIFT_MASK)
            snippet_info_jump_to_prev_placeholder(info, ec);
        return TRUE;
    } else {
        gchar* word = NULL;
        gchar* snippet = NULL;

        editor_get_current_iter(ec, &current);
        if (!gtk_text_iter_ends_word(&current)) return FALSE;

        start = current;
        gtk_text_iter_backward_word_start(&start);
        word = gtk_text_iter_get_text(&start, &current);
        snippet = snippets_get_value(sc, word);
        if (!snippet) {
            g_free(word);
            return FALSE;
        }
        info = snippets_parse(snippet);
        snippet_info_initial_expand(info);
        gtk_text_buffer_delete(ec_sourcebuffer, &start, &current);
        gtk_text_buffer_insert(ec_sourcebuffer, &start, info->expanded, -1);
        gtk_text_buffer_set_modified(ec_sourcebuffer, TRUE);
        gtk_text_iter_backward_chars(&start, strlen(info->expanded));
        info->start_offset = gtk_text_iter_get_offset(&start);
        snippet_info_jump_to_next_placeholder(info, ec);
        g_free(word);
        ec->snippet_editing = TRUE;
        return TRUE;
    }
    return FALSE;
}

GuSnippetInfo* snippets_parse(char* snippet) {
    gint start, end;
    GError* err = NULL;
    gchar** results = NULL;
    GRegex* holder_regex = NULL;
    GMatchInfo* match_info = NULL;
    const gchar* holder = "\\${?([0-9]*):?([^}]*)}?";

    GuSnippetInfo* info = snippet_info_new(snippet);

    if (!(holder_regex = g_regex_new(holder, G_REGEX_DOTALL, 0, &err))) {
        slog(L_ERROR, "g_regex_new(): %s\n", err->message);
        goto cleanup;
    }
    g_regex_match(holder_regex, snippet, 0, &match_info);
    while (g_match_info_matches(match_info)) {
        results = g_match_info_fetch_all(match_info);
        g_match_info_fetch_pos(match_info, 0, &start, &end);
        snippet_info_append_holder(info, atoi(results[1]), start, end -start, 
                results[2]);
        g_match_info_next(match_info, NULL);
    }
    g_strfreev(results);

    info->einfo_sorted = g_list_copy(info->einfo);
    info->einfo_sorted = g_list_sort(info->einfo_sorted, snippet_info_cmp);

cleanup:
    if (err) g_error_free(err);
    g_regex_unref(holder_regex);
    g_match_info_free(match_info);
    return info;
}

GuSnippetInfo* snippet_info_new(gchar* snippet) {
    GuSnippetInfo* info = g_new0(GuSnippetInfo, 1);
    info->snippet = g_strdup(snippet);
    info->expanded = g_strdup(snippet);
    info->einfo = NULL;
    info->einfo_sorted = NULL;
    return info;
}

void snippet_info_free(GuSnippetInfo* info) {
    GList* current = g_list_first(info->einfo);
    while (current) {
        g_free(GU_SNIPPET_EXPAND_INFO(current->data)->text);
        current = g_list_next(current);
    }
    g_list_free(info->einfo);
    g_list_free(info->einfo_sorted);
}

void snippet_info_jump_to_next_placeholder(GuSnippetInfo* info, GuEditor* ec) {
    GuSnippetExpandInfo* einfo = NULL;
    GtkTextIter start, current;
    if (!info->current) {
        if (GU_SNIPPET_EXPAND_INFO(info->einfo_unique->data)->group_number == 0)
            info->current = g_list_next(info->einfo_unique);
        else
            info->current = info->einfo_unique;
    } else {
        info->current = g_list_next(info->current);
    }
    if (!info->current) return;
    einfo = GU_SNIPPET_EXPAND_INFO(info->current->data);
    gtk_text_buffer_get_iter_at_offset(ec_sourcebuffer, &start,
            info->start_offset + einfo->start);
    current = start;
    gtk_text_iter_forward_chars(&current, einfo->len);
    gtk_text_buffer_place_cursor(ec_sourcebuffer, &start);
    gtk_text_buffer_select_range(ec_sourcebuffer, &start, &current);
}

void snippet_info_jump_to_prev_placeholder(GuSnippetInfo* info, GuEditor* ec) {
    GuSnippetExpandInfo* einfo = NULL;
    GtkTextIter start, current;
    if (!info->current)
        return;
    else
        info->current = g_list_previous(info->current);
    einfo = GU_SNIPPET_EXPAND_INFO(info->current->data);
    gtk_text_buffer_get_iter_at_offset(ec_sourcebuffer, &start,
            info->start_offset + einfo->start);
    gtk_text_buffer_place_cursor(ec_sourcebuffer, &start);
    current = start;
    gtk_text_iter_forward_chars(&current, einfo->len);
    gtk_text_buffer_select_range(ec_sourcebuffer, &start, &current);
}

void snippet_info_append_holder(GuSnippetInfo* info, gint group, gint start,
        gint len, gchar* text) {
    GuSnippetExpandInfo* einfo = g_new0(GuSnippetExpandInfo, 1);
    *einfo = (GuSnippetExpandInfo){ group, start, len, g_strdup(text) };
    info->einfo = g_list_append(info->einfo, einfo);
}

void snippet_info_initial_expand(GuSnippetInfo* info) {
    GHashTable* map = g_hash_table_new(NULL, NULL);
    GList* current = g_list_first(info->einfo);
    gint key = 0;
    GuSnippetExpandInfo* value = NULL;

    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        if (!g_hash_table_lookup_extended(map, ((gpointer)einfo->group_number),
                    (gpointer)&key, (gpointer)&value)) {
            g_hash_table_insert(map, (gpointer)einfo->group_number, einfo);
            info->einfo_unique = g_list_append(info->einfo_unique, einfo);
        }
        current = g_list_next(current);
    }
    info->einfo_unique = g_list_sort(info->einfo_unique, snippet_info_cmp);

    current = g_list_first(info->einfo);
    info->offset = 0;
    while (current) {
        GuSnippetExpandInfo* einfo = GU_SNIPPET_EXPAND_INFO(current->data);
        g_hash_table_lookup_extended(map, (gpointer)einfo->group_number,
                (gpointer)&key, (gpointer)&value);
        snippet_info_sub(info, einfo, value);
        current = g_list_next(current);
    }

    g_hash_table_destroy(map);
}

void snippet_info_sub(GuSnippetInfo* info, GuSnippetExpandInfo* target,
        GuSnippetExpandInfo* source) {
    gchar* first = NULL;
    gchar* third = NULL;
    gchar* new = NULL;

    target->start += info->offset;

    first = g_strndup(info->expanded, target->start);
    third = g_strdup(info->expanded + target->start + target->len);
    new = g_strconcat(first, source->text, third, NULL);
    g_free(info->expanded);
    info->expanded = new;

    info->offset += strlen(source->text) - target->len;
    target->len = strlen(source->text);
    if (source->text != target->text)
        g_free(target->text);
    target->text = g_strdup(source->text);
}

gint snippet_info_cmp(gconstpointer a, gconstpointer b) {
    return ((GU_SNIPPET_EXPAND_INFO(a)->group_number <
                GU_SNIPPET_EXPAND_INFO(b)->group_number)? -1: 1);
}
