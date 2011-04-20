/**
 * @file   gui-main.h
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

#define RECENT_FILES_NUM 5
#define TEXCOUNT_OUTPUT_LINES 7

/* These macros should be only used in GUI related classes 
 * which acted as syntax sugar */
#define g_e_buffer GTK_TEXT_BUFFER (g_active_editor->buffer)
#define g_e_view GTK_TEXT_VIEW (g_active_editor->view)
#define g_active_tab gui->tabmanagergui->active_tab
#define g_active_editor gui->tabmanagergui->active_editor
#define g_active_page gui->tabmanagergui->active_page



#define GUMMI_GUI(x) ((GummiGui*)x)
typedef struct _GummiGui GummiGui; 

struct _GummiGui {
    GuImportGui* importgui;
    GuPrefsGui* prefsgui;
    GuPreviewGui* previewgui;
    GuSearchGui* searchgui;
    GuSnippetsGui* snippetsgui;
    GuTabmanagerGui* tabmanagergui;

    GtkWindow *mainwindow;
    GtkTextBuffer *errorbuff;
    GtkVBox* rightpane;
    GtkHBox* toolbar;
    GtkStatusbar *statusbar;
    GtkToggleToolButton* previewoff;
    GtkCheckMenuItem* menu_spelling;
    GtkCheckMenuItem* menu_snippets;
    GtkCheckMenuItem* menu_toolbar;
    GtkCheckMenuItem* menu_statusbar;
    GtkCheckMenuItem* menu_rightpane;
    GtkMenuItem* recent[5];
    
    guint statusid;
    gchar* recent_list[5];
};

typedef enum _OpenAct {
    A_NONE = 0,
    A_DEFAULT,
    A_LOAD,
    A_LOAD_OPT,
} OpenAct;

typedef enum _GuFilterType {
    TYPE_LATEX = 0,
    TYPE_LATEX_SAVEAS,
    TYPE_PDF,
    TYPE_IMAGE,
    TYPE_BIBLIO
} GuFilterType;

/* Main GUI */
GummiGui* gui_init (GtkBuilder* builder);
void gui_main (GtkBuilder* builder);
gboolean gui_quit (void);
void gui_create_environment (OpenAct act, const gchar* filename,
                             const gchar* opt);
void gui_update_environment (const gchar* filename);
void gui_update_windowtitle (void);
void gui_open_file (const gchar* filename);
void gui_save_file (gboolean saveas);
void on_menu_close_activate (GtkWidget *widget, void* user);
void on_menu_new_activate (GtkWidget* widget, void* user);
void on_menu_open_activate (GtkWidget* widget, void* user);
void on_menu_save_activate (GtkWidget* widget, void* user);
void on_menu_saveas_activate (GtkWidget* widget, void* user);
void on_menu_find_activate (GtkWidget* widget, void* user);
void on_menu_cut_activate (GtkWidget* widget, void* user);
void on_menu_copy_activate (GtkWidget* widget, void* user);
void on_menu_paste_activate (GtkWidget* widget, void* user);
void on_menu_undo_activate (GtkWidget* widget, void* user);
void on_menu_redo_activate (GtkWidget* widget, void* user);
void on_menu_delete_activate (GtkWidget *widget, void* user);
void on_menu_selectall_activate (GtkWidget *widget, void* user);
void on_menu_preferences_activate (GtkWidget *widget, void* user);
void on_tab_notebook_switch_page(GtkNotebook *notebook, GtkWidget* nbpage,
        int page, void* data);
void on_menu_statusbar_toggled (GtkWidget *widget, void* user);
void on_menu_toolbar_toggled (GtkWidget *widget, void* user);
void on_menu_rightpane_toggled (GtkWidget *widget, void* user);
void on_menu_fullscreen_toggled (GtkWidget *widget, void* user);
void on_menu_find_activate (GtkWidget *widget, void* user);
void on_menu_findnext_activate (GtkWidget *widget, void* user);
void on_menu_findprev_activate (GtkWidget *widget, void* user);
void on_menu_bibload_activate (GtkWidget *widget, void* user);
void on_menu_bibupdate_activate (GtkWidget *widget, void* user);
void on_menu_pdfcompile_activate (GtkWidget *widget, void* user);
void on_menu_docstat_activate (GtkWidget *widget, void* user);
void on_menu_spelling_toggled (GtkWidget *widget, void* user);
void on_menu_update_activate (GtkWidget *widget, void* user);
void on_menu_about_activate (GtkWidget *widget, void* user);
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

void on_button_biblio_compile_clicked (GtkWidget* widget, void* user);
void on_button_biblio_detect_clicked (GtkWidget* widget, void* user);
void on_bibreference_clicked (GtkTreeView* view, GtkTreePath* Path,
        GtkTreeViewColumn* column, void* user);
void on_biblio_filter_changed (GtkWidget* widget, void* user);
gboolean on_bibprogressbar_update (void* user);

/* misc functions */
gchar* get_open_filename (GuFilterType type);
gchar* get_save_filename (GuFilterType type);
void file_dialog_set_filter (GtkFileChooser* dialog, GuFilterType type);
gint check_for_save (void);

void add_to_recent_list (const gchar* filename);
void display_recent_files (GummiGui* gui);

void errorbuffer_set_text (gchar *message);
void statusbar_set_message (gchar* message);
gboolean statusbar_del_message (void* user);

void check_preview_timer (void);

#endif /* __GUMMI_GUI_MAIN_H__ */
