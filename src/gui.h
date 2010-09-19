/**
 * @file    gui.h
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


#ifndef GUMMI_GUI_H
#define GUMMI_GUI_H

#include <gtk/gtk.h>

#define g_e_buffer GTK_TEXT_BUFFER(gummi->editor->sourcebuffer)
#define g_e_view GTK_TEXT_VIEW(gummi->editor->sourceview)

typedef struct _GuPrefsGui {
    GtkWidget* prefwindow;
    GtkNotebook* notebook;
    GtkCheckButton* textwrap_button;
    GtkCheckButton* wordwrap_button;
    GtkCheckButton* line_numbers;
    GtkCheckButton* highlighting;
    GtkCheckButton* autosaving;
    GtkCheckButton* compile_status;
    GtkSpinButton* tabwidth;
    GtkCheckButton* spaces_instof_tabs;
    GtkCheckButton* autoindentation;
    GtkSpinButton* autosave_timer;
    GtkComboBox* combo_languages;
    GtkListStore* list_languages;
    GtkTextView* default_text;
    GtkTextBuffer* default_buffer;
    GtkComboBox* typesetter;
    GtkFontButton* editor_font;
    GtkComboBox* compile_scheme;
    GtkSpinButton* compile_timer;
    GtkImage* changeimg;
    GtkLabel* changelabel;

    GtkVBox* view_box;
    GtkHBox* editor_box;
    GtkHBox* compile_box;
} GuPrefsGui;

typedef struct _GuSearchGui {
    GtkWidget* searchwindow;
    GtkEntry* searchentry;
    GtkEntry* replaceentry;
    gboolean backwards;
    gboolean matchcase;
    gboolean wholeword;
} GuSearchGui;

typedef struct _GuImportGui {
    GtkHBox* box_image;
    GtkHBox* box_table;
    GtkHBox* box_matrix;
    GtkViewport* image_pane;
    GtkViewport* table_pane;
    GtkViewport* matrix_pane;
} GuImportGui;

typedef struct _GummiGui {
    GuPrefsGui* prefsgui;
    GuSearchGui* searchgui;
    GuImportGui* importgui;

    GtkWidget *mainwindow;
    GtkTextBuffer *errorbuff;
    GtkVBox* rightpane;
    GtkHBox* toolbar;
    GtkStatusbar *statusbar;
    GtkToggleToolButton* previewoff;
    GtkCheckMenuItem* menu_spelling;
    GtkCheckMenuItem* menu_toolbar;
    GtkCheckMenuItem* menu_statusbar;
    GtkCheckMenuItem* menu_rightpane;
    GtkMenuItem* recent[5];
    
    guint statusid;
    gchar* recent_list[5];
} GummiGui;

typedef enum _GuFilterType {
    FILTER_LATEX = 0,
    FILTER_PDF,
    FILTER_IMAGE,
    FILTER_BIBLIO
} GuFilterType;

/* Main GUI */
GummiGui* gui_init(GtkBuilder* builder);
void gui_main(GtkBuilder* builder);
gboolean gui_quit(void);
void gui_update_title(void);
void on_menu_new_activate(GtkWidget* widget, void* user);
void on_menu_open_activate(GtkWidget* widget, void* user);
void on_menu_save_activate(GtkWidget* widget, void* user);
void on_menu_saveas_activate(GtkWidget* widget, void* user);
void on_menu_find_activate(GtkWidget* widget, void* user);
void on_menu_cut_activate(GtkWidget* widget, void* user);
void on_menu_copy_activate(GtkWidget* widget, void* user);
void on_menu_paste_activate(GtkWidget* widget, void* user);
void on_menu_undo_activate(GtkWidget* widget, void* user);
void on_menu_redo_activate(GtkWidget* widget, void* user);
void on_menu_delete_activate(GtkWidget *widget, void * user);
void on_menu_selectall_activate(GtkWidget *widget, void * user);
void on_menu_preferences_activate(GtkWidget *widget, void * user);
void on_menu_statusbar_toggled(GtkWidget *widget, void * user);
void on_menu_toolbar_toggled(GtkWidget *widget, void * user);
void on_menu_rightpane_toggled(GtkWidget *widget, void * user);
void on_menu_fullscreen_toggled(GtkWidget *widget, void * user);
void on_menu_find_activate(GtkWidget *widget, void* user);
void on_menu_findnext_activate(GtkWidget *widget, void * user);
void on_menu_findprev_activate(GtkWidget *widget, void * user);
void on_menu_bibload_activate(GtkWidget *widget, void * user);
void on_menu_bibupdate_activate(GtkWidget *widget, void * user);
void on_menu_docstat_activate(GtkWidget *widget, void * user);
void on_menu_spelling_toggled(GtkWidget *widget, void * user);
void on_menu_update_activate(GtkWidget *widget, void * user);
void on_menu_about_activate(GtkWidget *widget, void * user);
void on_tool_previewoff_toggled(GtkWidget *widget, void * user);
void on_tool_textstyle_bold_activate(GtkWidget* widget, void* user);
void on_tool_textstyle_italic_activate(GtkWidget* widget, void* user);
void on_tool_textstyle_underline_activate(GtkWidget* widget, void* user);
void on_tool_textstyle_left_activate(GtkWidget* widget, void* user);
void on_tool_textstyle_center_activate(GtkWidget* widget, void* user);
void on_tool_textstyle_right_activate(GtkWidget* widget, void* user);
void on_button_template_add_clicked(GtkWidget* widget, void* user);
void on_button_template_remove_clicked(GtkWidget* widget, void* user);
void on_button_template_open_clicked(GtkWidget* widget, void* user);
void on_button_template_close_clicked(GtkWidget* widget, void* user);
gboolean on_button_searchwindow_close_clicked(GtkWidget* widget, void* user);
void on_button_searchwindow_find_clicked(GtkWidget* widget, void* user);
void on_button_searchwindow_replace_next_clicked(GtkWidget* widget, void* user);
void on_button_searchwindow_replace_all_clicked(GtkWidget* widget, void* user);
void on_import_tabs_switch_page(GtkNotebook* notebook, GtkNotebookPage* page,
        guint page_num, void* user);

void on_bibcompile_clicked(GtkWidget* widget, void* user);
void on_bibrefresh_clicked(GtkWidget* widget, void* user);
void on_bibreference_clicked(GtkTreeView* view, GtkTreePath* Path,
        GtkTreeViewColumn* column, void* user);
gboolean on_bibprogressbar_update(void* user);

void preview_next_page(GtkWidget* widget, void* user);
void preview_prev_page(GtkWidget* widget, void* user);
void preview_zoom_change(GtkWidget* widget, void* user);

/* Preference GUI */
GuPrefsGui* prefsgui_init(GummiGui* gui);
void prefsgui_main(void);
void prefsgui_set_current_settings(GuPrefsGui* prefs);
void toggle_linenumbers(GtkWidget* widget, void* user);
void toggle_highlighting(GtkWidget* widget, void* user);
void toggle_textwrapping(GtkWidget* widget, void* user);
void toggle_wordwrapping(GtkWidget* widget, void* user);
void toggle_compilestatus(GtkWidget* widget, void* user);
void toggle_spaces_instof_tabs(GtkWidget* widget, void* user);
void toggle_autosaving(GtkWidget* widget, void* user);
void on_prefs_close_clicked(GtkWidget* widget, void* user);
void on_prefs_reset_clicked(GtkWidget* widget, void* user);

/* Search Window */
GuSearchGui* searchgui_init(GtkBuilder* builder);
void on_toggle_matchcase_toggled(GtkWidget* widget, void* user);
void on_toggle_wholeword_toggled(GtkWidget* widget, void* user);
void on_toggle_backwards_toggled(GtkWidget* widget, void* user);
void on_searchgui_text_changed(GtkEditable* editable, void* user);
void on_tabwidth_value_changed(GtkWidget* widget, void* user);
void on_autosave_value_changed(GtkWidget* widget, void* user);
void on_compile_value_changed(GtkWidget* widget, void* user);
void on_editor_font_set(GtkWidget* widget, void* user);
void on_combo_typesetter_changed(GtkWidget* widget, void* user);
void on_combo_language_changed(GtkWidget* widget, void* user);
void on_combo_compilescheme_changed(GtkWidget* widget, void* user);

/* Import GUI */
GuImportGui* importgui_init(GtkBuilder* builder);
void on_button_import_table_apply_clicked(GtkWidget* widget, void* user);
void on_button_import_image_apply_clicked(GtkWidget* widget, void* user);
void on_button_import_matrix_apply_clicked(GtkWidget* widget, void* user);
void on_image_file_activate(void);


/* misc functions */
gchar* get_open_filename(GuFilterType type);
gchar* get_save_filename(GuFilterType type, gchar* default_path);
void file_dialog_set_filter(GtkFileChooser* dialog, GuFilterType type);
gint check_for_save(void);

void add_to_recent_list(gchar* filename);
void display_recent_files(GummiGui* gui);

void errorbuffer_set_text(gchar *message);
void statusbar_set_message(gchar* message);
gboolean statusbar_del_message(void* user);

/**
 * @brief "changed" signal callback for editor->sourcebuffer
 * Automatically check whether to start timer if buffer changed.
 * Also set_modified for buffer
 */
void check_motion_timer(void);

#endif /* GUMMI_GUI_H */
