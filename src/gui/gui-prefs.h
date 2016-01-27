/**
 * @file   gui-prefs.h
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

#ifndef __GUMMI_GUI_PREFS_H__
#define __GUMMI_GUI_PREFS_H__

#include <glib.h>
#include <gtk/gtk.h>

#define GU_PREFS_GUI(x) ((GuPrefsGui*)x)
typedef struct _GuPrefsGui GuPrefsGui;

struct _GuPrefsGui {
    GtkWidget* prefwindow;
    GtkNotebook* notebook;
    GtkCheckButton* textwrap_button;
    GtkCheckButton* wordwrap_button;
    GtkCheckButton* line_numbers;
    GtkCheckButton* highlighting;
    GtkCheckButton* autosaving;
    GtkCheckButton* autoexport;
    GtkCheckButton* compile_status;
    GtkToggleButton* opt_shellescape;
    GtkToggleButton* opt_synctex;
    GtkSpinButton* tabwidth;
    GtkCheckButton* spaces_instof_tabs;
    GtkCheckButton* autoindentation;
    GtkSpinButton* autosave_timer;
    GtkComboBox* combo_languages;
    GtkListStore* list_languages;
    GtkTreeView* styleschemes_treeview;
    GtkListStore* list_styleschemes;
    GtkTextView* default_text;
    GtkTextBuffer* default_buffer;
    GtkFontButton* editor_font;
    GtkComboBox* compile_scheme;
    GtkSpinButton* compile_timer;
    GtkSpinButton* spin_cache_size;

    GtkToggleButton* typ_pdflatex;
    GtkToggleButton* typ_xelatex;
    GtkToggleButton* typ_rubber;
    GtkToggleButton* typ_latexmk;

    GtkToggleButton* method_texpdf;
    GtkToggleButton* method_texdvipdf;
    GtkToggleButton* method_texdvipspdf;

    GtkComboBox* combo_animated_scroll;

    GtkVBox* view_box;
    GtkHBox* editor_box;
    GtkHBox* compile_box;
};

GuPrefsGui* prefsgui_init (GtkWindow* mainwindow);
void prefsgui_main (GuPrefsGui* prefs, int page);
void prefsgui_apply_style_scheme(GuPrefsGui* prefs);
void toggle_linenumbers (GtkWidget* widget, void* user);
void toggle_highlighting (GtkWidget* widget, void* user);
void toggle_textwrapping (GtkWidget* widget, void* user);
void toggle_wordwrapping (GtkWidget* widget, void* user);
void toggle_compilestatus (GtkWidget* widget, void* user);
void toggle_spaces_instof_tabs (GtkWidget* widget, void* user);
void toggle_autosaving (GtkWidget* widget, void* user);
void on_prefs_close_clicked (GtkWidget* widget, void* user);
void on_prefs_reset_clicked (GtkWidget* widget, void* user);
void on_tabwidth_value_changed (GtkWidget* widget, void* user);
void on_configure_snippets_clicked (GtkWidget* widget, void* user);
void on_autosave_value_changed (GtkWidget* widget, void* user);
void on_compile_value_changed (GtkWidget* widget, void* user);
void on_editor_font_set (GtkWidget* widget, void* user);
void on_combo_typesetter_changed (GtkWidget* widget, void* user);
void on_combo_language_changed (GtkWidget* widget, void* user);
void on_combo_compilescheme_changed (GtkWidget* widget, void* user);
void on_combo_animated_scroll_changed (GtkWidget* widget, void* user);
void on_cache_size_value_changed(GtkWidget* widget, void* user);
void on_styleschemes_treeview_cursor_changed (GtkTreeView* treeview, void* user);





#endif /* __GUMMI_GUI_PREFS_H__ */
