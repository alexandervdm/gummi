/**
 * @file    editor.h
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


#ifndef GUMMI_EDITOR_H
#define GUMMI_EDITOR_H

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourceview.h>

#define ec_sourcebuffer GTK_TEXT_BUFFER(ec->sourcebuffer)
#define ec_sourceview GTK_TEXT_VIEW(ec->sourceview)

typedef struct _GuEditor {
    GtkSourceView *sourceview;
    GtkSourceBuffer *sourcebuffer;
    GtkTextTag* errortag;
    GtkTextTag* searchtag;
    GtkTextTagTable* editortags;
    gboolean replace_activated;
    gchar* term;
    gboolean backwards;
    gboolean wholeword;
    gboolean matchcase;
    gboolean cur_swap;
} GuEditor;

GuEditor* editor_init(GtkBuilder *builder);
void editor_sourceview_config(GuEditor* ec);
#ifdef USE_GTKSPELL
void editor_activate_spellchecking(GuEditor* ec, gboolean status);
#endif
void editor_fill_buffer(GuEditor* ec, const gchar* text);
gchar* editor_grab_buffer(GuEditor* ec);
void editor_insert_package(GuEditor* ec, const gchar* package);
void editor_insert_bib(GuEditor* ec, const gchar* package);
void editor_set_selection_textstyle(GuEditor* ec, const gchar* type);
void editor_apply_errortags(GuEditor* ec, gint line);
void editor_jumpto_search_result(GuEditor* ec, gint direction);
void editor_start_search(GuEditor* ec, const gchar* term, gboolean backwards,
        gboolean wholeword, gboolean matchcase, gboolean cs);
void editor_apply_searchtag(GuEditor* ec);
void editor_search_next(GuEditor* ec, gboolean inverse);
void editor_start_replace_next(GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase);
void editor_start_replace_all(GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase);
void editor_get_current_iter(GuEditor* ec, GtkTextIter* current);
inline void editor_scroll_to_cursor(GuEditor* ec);
void editor_undo_change(GuEditor* ec);
void editor_redo_change(GuEditor* ec);

#endif /* GUMMI_EDITOR_H */
