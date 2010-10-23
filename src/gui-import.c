/**
 * @file   gui-import.c
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

#include "gui-import.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "environment.h"
#include "utils.h"

extern Gummi* gummi;

GuImportGui* importgui_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GuImportGui* i = (GuImportGui*)g_malloc(sizeof(GuImportGui));
    i->box_image =
        GTK_HBOX(gtk_builder_get_object(builder, "box_image"));
    i->box_table =
        GTK_HBOX(gtk_builder_get_object(builder, "box_table"));
    i->box_matrix =
        GTK_HBOX(gtk_builder_get_object(builder, "box_matrix"));
    i->image_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "image_pane"));
    i->table_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "table_pane"));
    i->matrix_pane =
        GTK_VIEWPORT(gtk_builder_get_object(builder, "matrix_pane"));
    return i;
}

void on_import_tabs_switch_page(GtkNotebook* notebook, GtkNotebookPage* page,
        guint page_num, void* user) {
    GList* list = NULL;
    list = gtk_container_get_children(
            GTK_CONTAINER(gummi->gui->importgui->box_image));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(gummi->gui->importgui->box_image),
                GTK_WIDGET(list->data));
        list = list->next;
    }
    list = gtk_container_get_children(
            GTK_CONTAINER(gummi->gui->importgui->box_table));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(gummi->gui->importgui->box_table),
                GTK_WIDGET(list->data));
        list = list->next;
    }
    list = gtk_container_get_children(
            GTK_CONTAINER(gummi->gui->importgui->box_matrix));
    while (list) {
        gtk_container_remove(GTK_CONTAINER(gummi->gui->importgui->box_matrix),
                GTK_WIDGET(list->data));
        list = list->next;
    }

    switch (page_num) {
        case 1:
            gtk_container_add(GTK_CONTAINER(gummi->gui->importgui->box_image),
                    GTK_WIDGET(gummi->gui->importgui->image_pane));
            break;
        case 2:
            gtk_container_add(GTK_CONTAINER(gummi->gui->importgui->box_table),
                    GTK_WIDGET(gummi->gui->importgui->table_pane));
            break;
        case 3:
            gtk_container_add(GTK_CONTAINER(gummi->gui->importgui->box_matrix),
                    GTK_WIDGET(gummi->gui->importgui->matrix_pane));
            break;
    }
}

void on_button_import_table_apply_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    importer_insert_table(gummi->importer, gummi->editor);
}

void on_button_import_image_apply_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    importer_insert_image(gummi->importer, gummi->editor);
}

void on_button_import_matrix_apply_clicked(GtkWidget* widget, void* user) {
    L_F_DEBUG;
    importer_insert_matrix(gummi->importer, gummi->editor);
}

void on_image_file_activate(void) {
    L_F_DEBUG;
    gchar* filename = get_open_filename(TYPE_IMAGE);
    importer_imagegui_set_sensitive(gummi->importer, filename, TRUE);
    g_free(filename);
}
