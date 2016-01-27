/**
 * @file    utils.h
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


#ifndef __GUMMI_UTILS__
#define __GUMMI_UTILS__

#include <glib.h>
#include <gtk/gtk.h>

#ifdef WIN32
	#define DIR_PERMS (S_IRWXU)
#else
	#define DIR_PERMS (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif

#define TO_BOOL(X) ((X)? TRUE: FALSE)
#define STR_EQU(X, Y) (g_strcmp0((X), (Y)) == 0)

#define L_IS_TYPE(level, type) ((level & type) == type)
#define L_IS_GUI(level) (level & 0xf0)
#define L_INFO      0x00   /* for informative messages */
#define L_WARNING   0x01   /* warnning */
#define L_DEBUG     0x02   /* debug messages, only print if -d flags is used */
#define L_ERROR     0x04   /* reconverable error */
#define L_FATAL     0x08   /* inrecoverable error */
#define L_G_INFO    0x10   /* GUI info */
#define L_G_ERROR   0x20   /* recoverable error */
#define L_G_FATAL   0x40   /* inrecoverable error */

#define L_F_DEBUG slog(L_DEBUG, "%s ()\n", __func__);

/**
 * Tuple2:
 * @first: a gpointer that points to the first field
 * @second: a gpointer that points to the second field
 *
 * General purpose Tuple with 2 fields.
 */
typedef struct _Tuple2 {
    /*< private >*/
    struct _Tuple2* next;

    /*< public >*/
    gpointer first;
    gpointer second;
} Tuple2;

#define TUPLE2(x) ((Tuple2*)x)

/**
 * Tuple2:
 * @first: a gpointer that points to the first field
 * @second: a gpointer that points to the second field
 * @third: a gpointer that points to the third field
 *
 * General purpose Tuple with 3 fields.
 */
typedef struct _Tuple3 {
    /*< private >*/
    struct _Tuple3* next;

    /*< public >*/
    gpointer first;
    gpointer second;
    gpointer third;
} Tuple3;

#define TUPLE3(x) ((Tuple3*)x)

/**
 * slist:
 * @first: a gchar* that points to the key
 * @second: a gchar* that points to the value
 *
 * list for storing gummi settings, snippets.
 * Deprecated: Warning this sturct may be replaced with glist or Tuple2 in the
 * future.
 */
typedef struct _slist {
    /*< private >*/
    struct _slist* next;

    /*< public >*/
    gchar* first;
    gchar* second;
} slist;

void slog_init (gint debug);
gboolean in_debug_mode();
void slog_set_gui_parent (GtkWindow* p);
void slog (gint level, const gchar *fmt, ...);
gint utils_yes_no_dialog (const gchar* message);
gboolean utils_path_exists (const gchar* path);
gboolean utils_set_file_contents (const gchar *filename, const gchar *text,
        gssize length);

/**
 * utils_copy_file:
 *
 * Returns: return TRUE if succeed
 *
 * Platform independent file copy operation.
 */
gboolean utils_copy_file (const gchar* source, const gchar* dest, GError** err);

/**
 * utils_popen_r:
 *
 * Returns: A Tuple2 with Tuple2::first storing the exit code and
 * Tuple2::second pointing to a newly allocated gchar* array
 *
 * Platform independent interface for calling popen ().
 */
Tuple2 utils_popen_r (const gchar* cmd, const gchar* chdir);

/**
 * utils_path_to_relative:
 *
 * Returns: A newly allocated pointer to gchar* to the relative path, if target
 * isn't relative to root, target is simply duplicated and returned.
 *
 * Transforms target to path relative to root.
 */
gchar* utils_path_to_relative (const gchar* root, const gchar* target);

/**
 * utils_subinstr:
 *
 * Returns: A gboolean that states whether or the string in the first
 * argument is a substring of the second argument. When the case_sens arg
 * is passed as TRUE, case sensitivity of the two strings is ignored.
 */
gboolean utils_subinstr (const gchar* substr, const gchar* target,
        gboolean case_sens);

gchar* utils_get_tmp_tmp_dir (void); /* TODO: remove when we can */


gboolean utils_glist_is_member (GList *list, gchar* item);

gchar* g_substr(gchar* src, gint start, gint end);

/**
 * slist_find:
 * @head: the list head
 * @term: the term to find
 * @n: TRUE if only compare first n characters (using strncmp)
 * @create: TRUE to create new entry for term that isn't found
 *
 * Returns: a pointer to the slist node
 *
 * Find term in slist.
 */
slist* slist_find (slist* head, const gchar* term, gboolean n, gboolean create);

slist* slist_append (slist* head, slist* node);
slist* slist_remove (slist* head, slist* node);

#endif /* __GUMMI_UTILS__ */
