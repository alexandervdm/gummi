/**
 * @file   gui-main.h
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


#ifndef __GUMMI_GUI_MAIN_H__
#define __GUMMI_GUI_MAIN_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "gui-import.h"
#include "gui-prefs.h"
#include "gui-preview.h"
#include "gui-search.h"
#include "gui-snippets.h"
#include "gui-tabmanager.h"
#include "gui-infoscreen.h"
#include "gui-menu.h"
#include "gui-project.h"

#define RECENT_FILES_NUM 5
#define TEXCOUNT_OUTPUT_LINES 7

/* These macros should be only used in GUI related classes
 * which acted as syntax sugar */
/* These macros should be only used in GUI related classes
 * which acted as syntax sugar */
#define g_e_buffer GTK_TEXT_BUFFER (g_active_editor->buffer)
#define g_e_view GTK_TEXT_VIEW (g_active_editor->view)
#define g_active_tab gummi->tabmanager->active_tab
#define g_active_editor gummi->tabmanager->active_editor
#define g_tabs gummi->tabmanager->tabs


#define GUMMI_GUI(x) ((GummiGui*)x)
typedef struct _GummiGui GummiGui;

struct _GummiGui {
    GuMenuGui* menugui;
    GuImportGui* importgui;
    GuPrefsGui* prefsgui;
    GuPreviewGui* previewgui;
    GuSearchGui* searchgui;
    GuSnippetsGui* snippetsgui;
    GuTabmanagerGui* tabmanagergui;
    GuInfoscreenGui* infoscreengui;
    GuProjectGui* projectgui;

    GtkWindow* mainwindow;
    GtkTextBuffer* errorbuff;
    GtkTextView* errorview;
        GtkVBox* rightpane;

    GtkWidget* toolbar;
    GtkStatusbar* statusbar;
    GtkToggleToolButton* previewoff;
    GtkCheckMenuItem* menu_spelling;
    GtkCheckMenuItem* menu_snippets;
    GtkCheckMenuItem* menu_toolbar;
    GtkCheckMenuItem* menu_statusbar;
    GtkCheckMenuItem* menu_rightpane;
    GtkCheckMenuItem* menu_autosync;
    GtkMenuItem* recent[5];
    gint insens_widget_size;
    GtkWidget** insens_widgets;
    GtkBuilder *builder;
    GtkWidget *docstatswindow;
    GtkWidget *bibcompile;

    GtkMenuItem* menu_runbibtex;
    GtkMenuItem* menu_runmakeindex;

    #ifdef WIN32
		GtkWidget* w32label;
		GtkWidget* w32button;
		GtkWidget* w32window;
    #endif

    guint statusid;
    gchar* recent_list[5];
};

typedef enum _GuFilterType {
    TYPE_LATEX = 0,
    TYPE_LATEX_SAVEAS,
    TYPE_PDF,
    TYPE_IMAGE,
    TYPE_BIBLIO,
    TYPE_PROJECT
} GuFilterType;

/* Main GUI */
GummiGui* gui_init (GtkBuilder* builder);
void gui_main (GtkBuilder* builder);
gboolean gui_quit (void);


void gui_set_filename_display (GuTabContext* tc,
                                        gboolean title, gboolean label);
void gui_set_window_title (const gchar* filename, const gchar* text);


void gui_open_file (const gchar* filename);
void gui_save_file (GuTabContext* tab, gboolean saveas);
void gui_set_hastabs_sensitive (gboolean enable);

void on_tab_notebook_switch_page(GtkNotebook *notebook, GtkWidget* nbpage,
        int page, void* data);
void on_tool_previewoff_toggled (GtkWidget *widget, void * user);
void on_tool_textstyle_bold_activate (GtkWidget* widget, void* user);
void on_tool_textstyle_italic_activate (GtkWidget* widget, void* user);
void on_tool_textstyle_underline_activate (GtkWidget* widget, void* user);
void on_tool_textstyle_left_activate (GtkWidget* widget, void* user);
void on_tool_textstyle_center_activate (GtkWidget* widget, void* user);
void on_tool_textstyle_right_activate (GtkWidget* widget, void* user);
void on_button_template_add_clicked (GtkWidget* widget, void* user);
void on_button_template_remove_clicked (GtkWidget* widget, void* user);
void on_button_template_open_clicked (GtkWidget* widget, void* user);
void on_button_template_close_clicked (GtkWidget* widget, void* user);
void on_template_rowitem_edited (GtkWidget* widget, gchar *path, gchar* filenm,
        void* user);
void gui_recover_from_swapfile (const gchar* filename);
void on_menu_autosync_toggled (GtkCheckMenuItem *menu_autosync, void* user);

void on_button_biblio_compile_clicked (GtkWidget* widget, void* user);
void on_button_biblio_detect_clicked (GtkWidget* widget, void* user);
void on_bibreference_clicked (GtkTreeView* view, GtkTreePath* Path,
        GtkTreeViewColumn* column, void* user);
void on_biblio_filter_changed (GtkWidget* widget, void* user);
gboolean on_bibprogressbar_update (void* user);

void on_recovery_infobar_response (GtkInfoBar* bar, gint res, gpointer filename);
void gui_recovery_mode_enable (GuTabContext* tab, const gchar* filename);
void gui_recovery_mode_disable (GtkInfoBar *infobar);

/* misc functions */
gchar* get_open_filename (GuFilterType type);
gchar* get_save_filename (GuFilterType type);
void file_dialog_set_filter (GtkFileChooser* dialog, GuFilterType type);
gint check_for_save (GuEditor* editor);

void add_to_recent_list (const gchar* filename);
void display_recent_files (GummiGui* gui);

void gui_buildlog_set_text (const gchar *message);
void statusbar_set_message (const gchar* message);
gboolean statusbar_del_message (void* user);

void typesetter_setup (void);

void check_preview_timer (void);

#endif /* __GUMMI_GUI_MAIN_H__ */
