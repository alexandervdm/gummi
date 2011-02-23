

#include "gui-editortabs.h"

#include <gtk/gtk.h>

#include "environment.h"
#include "gui/gui-main.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuEditortabsGui* editortabsgui_init(GtkBuilder* builder) {
    g_return_val_if_fail(GTK_IS_BUILDER(builder), NULL);
    
    GuEditortabsGui* et = g_new0(GuEditortabsGui, 1);
    
    GtkWidget *scroll = 
            GTK_WIDGET(gtk_builder_get_object(builder, "tab_default_scroll"));
    gtk_container_add(
            GTK_CONTAINER(scroll), GTK_WIDGET(gummi->editor->sourceview));

    return et;
}



