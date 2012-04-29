/**
 * @file   gui-import.c
 * @brief
 *
 * Copyright (C) 2009-2012 Gummi-Dev Team <alexvandermey@gmail.com>
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

#include "gui-import.h"

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "environment.h"
#include "utils.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuImportGui* importgui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuImportGui* i = g_new0 (GuImportGui, 1);
    
    i->import_panel =
        GTK_HBOX (gtk_builder_get_object (builder, "import_panel"));
        
    i->image_pane =
        GTK_VIEWPORT (gtk_builder_get_object (builder, "imp_pane_image"));
    i->table_pane =
        GTK_VIEWPORT (gtk_builder_get_object (builder, "imp_pane_table"));
    i->matrix_pane =
        GTK_VIEWPORT (gtk_builder_get_object (builder, "imp_pane_matrix"));
    i->biblio_pane = 
        GTK_VIEWPORT (gtk_builder_get_object (builder, "imp_pane_biblio"));

    i->image_file =
        GTK_ENTRY (gtk_builder_get_object (builder, "image_file"));
    i->image_caption =
        GTK_ENTRY (gtk_builder_get_object (builder, "image_caption"));
    i->image_label =
        GTK_ENTRY (gtk_builder_get_object (builder, "image_label"));
    i->image_scale =
        GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "image_scale"));
    i->scaler =
        GTK_ADJUSTMENT (gtk_builder_get_object (builder, "image_scaler"));

    i->table_comboalign =
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "table_comboalign"));
    i->table_comboborder =
        GTK_COMBO_BOX (gtk_builder_get_object (builder, "table_comboborder"));
    i->table_rows =
        GTK_ADJUSTMENT (gtk_builder_get_object (builder, "table_rows"));
    i->table_cols =
        GTK_ADJUSTMENT (gtk_builder_get_object (builder, "table_cols"));

    i->matrix_rows =
        GTK_ADJUSTMENT (gtk_builder_get_object (builder, "matrix_rows"));
    i->matrix_cols =
        GTK_ADJUSTMENT (gtk_builder_get_object (builder, "matrix_cols"));
    i->matrix_combobracket =
        GTK_COMBO_BOX (gtk_builder_get_object (builder,"matrix_combobracket"));
        
    i->biblio_file = 
         GTK_ENTRY (gtk_builder_get_object (builder, "biblio_file"));   

    gtk_adjustment_set_value (i->table_cols, 3);
    gtk_adjustment_set_value (i->table_rows, 3);
    gtk_adjustment_set_value (i->matrix_cols, 3);
    gtk_adjustment_set_value (i->matrix_rows, 3);
    return i;
}

/*
G_MODULE_EXPORT
void on_import_tabs_switch_page (GtkNotebook* notebook, GtkNotebookPage* page,
        guint page_num, void* user) {
    GList* list = NULL;
    list = gtk_container_get_children (
            GTK_CONTAINER (g_importgui->box_image));
            

    while (list) {
        gtk_container_remove (GTK_CONTAINER (g_importgui->box_image),
                GTK_WIDGET (list->data));
        list = list->next;
    }
    list = gtk_container_get_children (
            GTK_CONTAINER (g_importgui->box_table));
    while (list) {
        gtk_container_remove (GTK_CONTAINER (g_importgui->box_table),
                GTK_WIDGET (list->data));
        list = list->next;
    }
    list = gtk_container_get_children (
            GTK_CONTAINER (g_importgui->box_matrix));
    while (list) {
        gtk_container_remove (GTK_CONTAINER (g_importgui->box_matrix),
                GTK_WIDGET (list->data));
        list = list->next;
    }
    list = gtk_container_get_children (
            GTK_CONTAINER (g_importgui->box_biblio));
    while (list) {
        gtk_container_remove (GTK_CONTAINER (g_importgui->box_biblio),
                GTK_WIDGET (list->data));
        list = list->next;
    }

    switch (page_num) {
        case 1:
            gtk_container_add (GTK_CONTAINER (g_importgui->box_image),
                    GTK_WIDGET (g_importgui->image_pane));
            break;
        case 2:
            gtk_container_add (GTK_CONTAINER (g_importgui->box_table),
                    GTK_WIDGET (g_importgui->table_pane));
            break;
        case 3:
            gtk_container_add (GTK_CONTAINER (g_importgui->box_matrix),
                    GTK_WIDGET (g_importgui->matrix_pane));
        case 4:
            gtk_container_add (GTK_CONTAINER (g_importgui->box_biblio),
                    GTK_WIDGET (g_importgui->biblio_pane));
            break;
    }
}
*/

void importgui_remove_all_panels () {
    GList* list = NULL;
    
    list = gtk_container_get_children (
            GTK_CONTAINER (g_importgui->import_panel));
    while (list) {
        gtk_container_remove (GTK_CONTAINER (g_importgui->import_panel),
                GTK_WIDGET (list->data));
        list = list->next;
    }
}

G_MODULE_EXPORT
void on_imp_panel_image_clicked (GtkWidget* widget, void* user) {
    importgui_remove_all_panels ();
    gtk_container_add (GTK_CONTAINER (g_importgui->import_panel),
                    GTK_WIDGET (g_importgui->image_pane));
}

G_MODULE_EXPORT
void on_imp_panel_table_clicked (GtkWidget* widget, void* user) {
    importgui_remove_all_panels ();
    gtk_container_add (GTK_CONTAINER (g_importgui->import_panel),
                    GTK_WIDGET (g_importgui->table_pane));
}

G_MODULE_EXPORT
void on_imp_panel_matrix_clicked (GtkWidget* widget, void* user) {
    importgui_remove_all_panels ();
    gtk_container_add (GTK_CONTAINER (g_importgui->import_panel),
                    GTK_WIDGET (g_importgui->matrix_pane));
}

G_MODULE_EXPORT
void on_imp_panel_biblio_clicked (GtkWidget* widget, void* user) {
    importgui_remove_all_panels ();
    gtk_container_add (GTK_CONTAINER (g_importgui->import_panel),
                    GTK_WIDGET (g_importgui->biblio_pane));
}

G_MODULE_EXPORT
void on_imp_minimize_clicked (GtkWidget* widget, void* user) {
    importgui_remove_all_panels ();
}

G_MODULE_EXPORT
void on_import_table_apply_clicked (GtkWidget* widget, void* user) {
    GtkTextIter current;
    gint rows = gtk_adjustment_get_value (g_importgui->table_rows);
    gint cols = gtk_adjustment_get_value (g_importgui->table_cols);
    gint border = gtk_combo_box_get_active (g_importgui->table_comboborder);
    gint align = gtk_combo_box_get_active (g_importgui->table_comboalign);
    const gchar* text = importer_generate_table (rows, cols, border, align);

    editor_get_current_iter (g_active_editor, &current);
    gtk_text_buffer_begin_user_action (g_e_buffer);
    gtk_text_buffer_insert (g_e_buffer, &current, text, strlen (text));
    gtk_text_buffer_end_user_action (g_e_buffer);
    gtk_text_buffer_set_modified (g_e_buffer, TRUE);
    importgui_remove_all_panels ();
}

G_MODULE_EXPORT
void on_import_image_apply_clicked (GtkWidget* widget, void* user) {
    GtkTextIter current;
    const gchar* imagefile = gtk_entry_get_text (g_importgui->image_file);
    const gchar* caption = gtk_entry_get_text (g_importgui->image_caption);
    const gchar* label = gtk_entry_get_text (g_importgui->image_label);
    gdouble scale = gtk_adjustment_get_value (g_importgui->scaler);
    gchar* root_path = NULL;
    gchar* relative_path = NULL;
    const gchar* text = 0;

    if (0 != strlen (imagefile)) {
        if (!utils_path_exists (imagefile)) {
            slog (L_G_ERROR, _("%s: No such file or directory\n"), imagefile);
        } else {
            if (g_active_editor->filename)
                root_path = g_path_get_dirname (g_active_editor->filename);
            relative_path = utils_path_to_relative (root_path, imagefile);
            text =importer_generate_image(relative_path, caption, label, scale);
            editor_insert_package (g_active_editor, "graphicx");
            editor_get_current_iter (g_active_editor, &current);
            gtk_text_buffer_begin_user_action (g_e_buffer);
            gtk_text_buffer_insert (g_e_buffer, &current,text,strlen (text));
            gtk_text_buffer_end_user_action (g_e_buffer);
            gtk_text_buffer_set_modified (g_e_buffer, TRUE);
            importer_imagegui_set_sensitive ("", FALSE);
        }
    }
    importgui_remove_all_panels ();
    g_free (relative_path);
    g_free (root_path);
}

G_MODULE_EXPORT
void on_import_matrix_apply_clicked (GtkWidget* widget, void* user) {
    GtkTextIter current;
    gint bracket =
        gtk_combo_box_get_active (g_importgui->matrix_combobracket);
    gint rows = gtk_adjustment_get_value (g_importgui->matrix_rows);
    gint cols = gtk_adjustment_get_value (g_importgui->matrix_cols);
    const gchar* text = importer_generate_matrix (bracket, rows, cols);
    editor_insert_package (g_active_editor, "amsmath");
    editor_get_current_iter (g_active_editor, &current);
    gtk_text_buffer_begin_user_action (g_e_buffer);
    gtk_text_buffer_insert (g_e_buffer, &current, text, strlen (text));
    gtk_text_buffer_end_user_action (g_e_buffer);
    gtk_text_buffer_set_modified (g_e_buffer, TRUE);
    importgui_remove_all_panels ();
}

G_MODULE_EXPORT
void on_import_biblio_apply_clicked (GtkWidget* widget, void* user) {
    gchar* basename = NULL;
    gchar* root_path = NULL;
    gchar* relative_path = NULL;
    
    const gchar* filename = gtk_entry_get_text (g_importgui->biblio_file);

    if ((filename) && (strlen(filename) != 0)) {
        if (g_active_editor->filename)
            root_path = g_path_get_dirname (g_active_editor->filename);
        relative_path = utils_path_to_relative (root_path, filename);
        editor_insert_bib (g_active_editor, relative_path);
        basename = g_path_get_basename (filename);
        gtk_label_set_text (gummi->biblio->filenm_label, basename);
        g_free (relative_path);
        g_free (root_path);
        g_free (basename);
        gtk_entry_set_text (g_importgui->biblio_file, "");
    }
    importgui_remove_all_panels ();
}

G_MODULE_EXPORT
void on_image_file_activate (void) {
    gchar* filename = NULL;
    
    filename = get_open_filename (TYPE_IMAGE);
    if (filename) {
        importer_imagegui_set_sensitive (filename, TRUE);
    }
    g_free (filename);
}

G_MODULE_EXPORT
void on_biblio_file_activate (GtkWidget *widget, void * user) {
    gchar* filename = NULL;
    
    filename = get_open_filename (TYPE_BIBLIO);
    if (filename) {
        gtk_entry_set_text (g_importgui->biblio_file, filename);
    }
    g_free (filename);
}

G_MODULE_EXPORT
void importer_imagegui_set_sensitive (const gchar* name, gboolean mode) {
    gtk_widget_set_sensitive (GTK_WIDGET (g_importgui->image_label), mode);
    gtk_widget_set_sensitive (GTK_WIDGET (g_importgui->image_caption), mode);
    gtk_widget_set_sensitive (GTK_WIDGET (g_importgui->image_scale), mode);
    gtk_entry_set_text (g_importgui->image_file, name);
    gtk_entry_set_text (g_importgui->image_label, "");
    gtk_entry_set_text (g_importgui->image_caption, "");
    gtk_adjustment_set_value (g_importgui->scaler, 1.00);
}
