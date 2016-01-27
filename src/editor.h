/**
 * @file    editor.h
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


#ifndef __GUMMI_EDITOR_H__
#define __GUMMI_EDITOR_H__

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "motion.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcestyleschememanager.h>
#include <gtksourceview/gtksourceview.h>

#define ec_buffer GTK_TEXT_BUFFER(ec->buffer)
#define ec_view GTK_TEXT_VIEW(ec->view)

#define GU_EDITOR(x) ((GuEditor*)x)
typedef struct _GuEditor GuEditor;

struct _GuEditor {
    /* File related members */
    gint workfd;
    gchar* fdname;
    gchar* filename;
    gchar* basename;
    gchar* pdffile;
    gchar* workfile;
    gchar* bibfile;
    gchar* projfile;

    /* GUI related members */
    GtkSourceView* view;
    GtkSourceBuffer* buffer;
    GtkSourceStyleSchemeManager* stylemanager;
    GtkTextTag* errortag;
    GtkTextTag* searchtag;
    GtkTextTagTable* editortags;
    gboolean replace_activated;
    gchar* term;
    gboolean backwards;
    gboolean wholeword;
    gboolean matchcase;
    gint sigid[5];

    GtkTextIter last_edit;
    gboolean sync_to_last_edit;
};

GuEditor* editor_new (GuMotion* mc);
void editor_fileinfo_update (GuEditor* ec, const gchar* filename);
void editor_fileinfo_cleanup (GuEditor* ec);
gboolean editor_fileinfo_update_biblio (GuEditor* ec,  const gchar* filename);
void editor_destroy (GuEditor* ec);
void editor_sourceview_config (GuEditor* ec);
#ifdef USE_GTKSPELL
void editor_activate_spellchecking (GuEditor* ec, gboolean status);
#endif
void editor_fill_buffer (GuEditor* ec, const gchar* text);

/* editor_grab_buffer will return a newly allocated string */
gchar* editor_grab_buffer (GuEditor* ec);
void editor_insert_package (GuEditor* ec, const gchar* package, const gchar* options);
void editor_insert_bib (GuEditor* ec, const gchar* package);
void editor_set_selection_textstyle (GuEditor* ec, const gchar* type);
void editor_apply_errortags (GuEditor* ec, gint* lines);
void editor_jumpto_search_result (GuEditor* ec, gint direction);
void editor_start_search (GuEditor* ec, const gchar* term, gboolean backwards,
        gboolean wholeword, gboolean matchcase);
void editor_apply_searchtag (GuEditor* ec);
void editor_search_next (GuEditor* ec, gboolean inverse);
void editor_start_replace_next (GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase);
void editor_start_replace_all (GuEditor* ec, const gchar* term,
        const gchar* rterm, gboolean backwards, gboolean wholeword,
        gboolean matchcase);
void editor_get_current_iter (GuEditor* ec, GtkTextIter* current);
inline void editor_scroll_to_cursor (GuEditor* ec);
void editor_scroll_to_line (GuEditor* ec, gint line);
void editor_undo_change (GuEditor* ec);
void editor_redo_change (GuEditor* ec);
void editor_set_style_scheme_by_id (GuEditor* ec, const gchar* id);
void set_style_fg_bg (GObject* obj, GtkSourceStyleScheme* scheme,
                      gchar* styleName, gchar* defaultBG);
gint schemes_compare (gconstpointer a, gconstpointer b);
GList* editor_list_style_scheme_sorted (void);
gboolean editor_buffer_changed (GuEditor* ec);

#endif /* __GUMMI_EDITOR_H__ */
