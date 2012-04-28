/**
 * @file   gui-tabmanager.c
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

#include "gui-tabmanager.h"

#include "gui-main.h"
#include "environment.h"

extern Gummi* gummi;
extern GummiGui* gui;

GuTabmanagerGui* tabmanagergui_init (GtkBuilder* builder) {
    g_return_val_if_fail (GTK_IS_BUILDER (builder), NULL);

    GuTabmanagerGui* tm = g_new0 (GuTabmanagerGui, 1);

    tm->notebook =
        GTK_NOTEBOOK (gtk_builder_get_object (builder, "tab_notebook"));
    g_object_set (tm->notebook, "tab-border", 0, NULL);
    
    tm->unsavednr = 0;
    return tm;
}

int tabmanagergui_create_page (GuTabContext* tc, GuEditor* editor) {
    GuTabPage* tp = g_new0(GuTabPage, 1);
    tc->page = tp;
    int pos;
    
    tp->scrollw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(tp->scrollw),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                                    
    gchar* labeltext = tabmanager_get_tabname (tc);
    tabmanagergui_create_label (tp, labeltext);
    g_signal_connect (tp->button, "clicked", 
                      G_CALLBACK (on_menu_close_activate), tc);
    tabmanagergui_create_infobar (tp);

    gtk_container_add (GTK_CONTAINER (tp->scrollw), 
                       GTK_WIDGET (editor->view));
                       
    tp->editorbox = gtk_vbox_new (FALSE, 0);

    gtk_box_pack_start (GTK_BOX (tp->editorbox), tp->infobar, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (tp->editorbox), tp->scrollw, TRUE, TRUE, 0);
    
    pos = gtk_notebook_append_page (GTK_NOTEBOOK (g_tabnotebook), 
                                    tp->editorbox, GTK_WIDGET (tp->labelbox));

    gtk_widget_show_all (tp->editorbox);
    return pos;
}

void tabmanagergui_create_infobar (GuTabPage* tp) {
    // we will probably want to make a separate file for infobar 
    // procedures that we can attach to hboxes in both the editor 
    // and the preview window, TODO for 0.7.0 -Alex
    GtkWidget* infobar = NULL;
    GtkWidget* message = NULL;
    GtkWidget* area = NULL;
    
    infobar = gtk_info_bar_new ();
    gtk_widget_set_no_show_all (infobar, TRUE);
    message = gtk_label_new ("");
    gtk_label_set_line_wrap (GTK_LABEL(message), TRUE);
    
    gtk_widget_show (message);
    area = gtk_info_bar_get_content_area (GTK_INFO_BAR (infobar));
    gtk_container_add (GTK_CONTAINER (area), message);
    
    gtk_info_bar_add_button (GTK_INFO_BAR (infobar),
                            GTK_STOCK_YES, GTK_RESPONSE_YES);
    gtk_info_bar_add_button (GTK_INFO_BAR (infobar),
                            GTK_STOCK_NO, GTK_RESPONSE_NO);
                  
    gtk_info_bar_set_message_type (GTK_INFO_BAR (infobar),
                                  GTK_MESSAGE_WARNING);
                                  
    tp->barlabel = message;
    tp->infobar = infobar;
}

void tabmanagergui_create_label (GuTabPage* tp, gchar* labeltext) {
    static unsigned count = 0;
    GtkRcStyle* rcstyle = NULL;
    GtkWidget* image = NULL;
    GtkHBox* hbox;

    tp->labelbox = gtk_event_box_new ();
    hbox = GTK_HBOX (gtk_hbox_new (FALSE, 0));
    tp->unsavednr = ++count;
    
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (tp->labelbox), FALSE);
    gtk_container_add (GTK_CONTAINER(tp->labelbox), GTK_WIDGET (hbox));
    
    tp->label = GTK_LABEL (gtk_label_new (labeltext));

    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (tp->label), TRUE, TRUE, 5);
    
    tp->button = GTK_BUTTON (gtk_button_new());
    image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
    gtk_button_set_image (tp->button, image);
    g_object_set (tp->button, "relief", GTK_RELIEF_NONE, 
                         "focus-on-click", FALSE, NULL);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (tp->button), FALSE,FALSE,0);
    
    rcstyle = gtk_rc_style_new ();
    rcstyle->xthickness = rcstyle->ythickness = 0;
    gtk_widget_modify_style (GTK_WIDGET (tp->button), rcstyle);
    g_object_unref (rcstyle);

    gtk_widget_show_all (GTK_WIDGET (hbox));
}

gchar* tabmanagergui_get_labeltext (GuTabPage* tp) {
    const gchar* text = gtk_label_get_text (GTK_LABEL(tp->label));
    return (gchar*)text;
}

gint tabmanagergui_replace_page (GuTabContext* tc, GuEditor* newec) {
    
    gummi->tabmanager->active_tab->editor = newec;
    
    gtk_container_remove (GTK_CONTAINER (tc->page->scrollw),
                          GTK_WIDGET (g_active_editor->view));
    editor_destroy (g_active_editor);
    gtk_container_add (GTK_CONTAINER (tc->page->scrollw),
                       GTK_WIDGET (newec->view));
    gtk_widget_show (GTK_WIDGET(newec->view));

    int pos = gtk_notebook_page_num (g_tabnotebook, 
                            gummi->tabmanager->active_tab->page->editorbox);
    return pos;
}

void tabmanagergui_set_current_page (gint position) {
    gtk_notebook_set_current_page (g_tabnotebook, position);
}

gint tabmanagergui_get_current_page (void) {
    return gtk_notebook_get_current_page (g_tabnotebook);
}

gint tabmanagergui_get_n_pages (void) {
    return gtk_notebook_get_n_pages (g_tabnotebook);
}

void tabmanagergui_update_label (GuTabPage* tp, const gchar* text) {
    g_return_if_fail (tp != NULL);
    gtk_label_set_text (tp->label, text);
    if (tp->bold) tablabel_set_bold_text (tp);
}

void tablabel_set_bold_text (GuTabPage* tp) {
    gchar* markup;
    const gchar* cur = gtk_label_get_text (tp->label);
    markup = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", cur);
    gtk_label_set_markup (GTK_LABEL (tp->label), markup);
    g_free (markup);
    if (!tp->bold) tp->bold = TRUE;
}
