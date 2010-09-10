/**
 * @file   template.c
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


#include "template.h"

#include <stdlib.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "environment.h"
#include "utils.h"

gchar* template_article = 
"\\documentclass{article}\n"
"\\author{[YOUR NAME]\\\\\n"
"\\texttt{[YOUR EMAIL]}\n"
"}\n"
"\\title{[TITLE OF YOUR ARTICLE]}\n"
"\\begin{document}\n"
"\\maketitle\n"
"\\dots\n"
"\n"
"\\end{document}\n";

gchar* template_book =
"\\documentclass[12pt]{book}\n"
"\n"
"\\begin{document}\n"
"\n"
"\\chapter*{\\Huge \\center [BOOKTITLE] }\n"
"\\thispagestyle{empty}\n"
"\\section*{\\huge \\center [AUTHOR]}\n"
"\\newpage\n"
"\n"
"\\subsection*{\\center \\normalsize Copyright \\copyright [YEAR] [NAME]}\n"
"\\subsection*{\\center \\normalsize All rights reserved.}\n"
"\\subsection*{\\center \\normalsize ISBN \\dots}\n"
"\\subsection*{\\center \\normalsize Publications}\n"
"\n"
"\\tableofcontents\n"
"\n"
"\\mainmatter\n"
"\\chapter{[CHAPTER1-TITLE]}\n"
"\\dots\n"
"\\chapter{[CHAPTER2-TITLE]}\n"
"\\dots\n"
"\\backmatter\n"
"\n"
"\\end{document}\n";

gchar* template_letter =
"\\documentclass{letter}\n"
"\n"
"\\signature{[YOURNAME]}\n"
"\\address{[YOURADDRESS]}\n"
"\n"
"\\begin{document}\n"
"\\begin{letter}{Company name \\\\ Street\\\\ City\\\\ Country}\n"
"\\opening{[HEADING]}\n"
"\n"
"\\dots\\\\\\dots\\\\\\dots\\\\\\dots\\\\\\dots\n"
"\n"
"\\closing{[CLOSING]}\n"
"\\end{letter}\n"
"\\end{document}\n";

gchar* template_report =
"\\documentclass[]{report}\n"
"\\begin{document}\n"
"\n"
"\\title{[YOUR TITLE]}\n"
"\\author{[YOUR NAME]}\n"
"\\maketitle\n"
"\n"
"\\chapter{[CHAPTERTITLE]}\n"
"\\section{Introduction}\n"
"\\dots\n"
"\\chapter{[CHAPTERTITLE]}\n"
"\\section{Introduction}\n"
"\\dots\n"
"\\subsection{Subsection}\n"
"\\end{document}\n";

GuTemplate* template_init(GtkBuilder* builder) {
    L_F_DEBUG;
    GuTemplate* t = (GuTemplate*)g_malloc(sizeof(GuTemplate));
    t->templatewindow =
        GTK_WINDOW(gtk_builder_get_object(builder, "templatewindow"));
    t->iconview =
        GTK_ICON_VIEW(gtk_builder_get_object(builder, "templateicons"));
    t->template_ok =
        GTK_BUTTON(gtk_builder_get_object(builder, "template_ok"));
    gtk_icon_view_set_text_column(t->iconview, 0);
    gtk_icon_view_set_pixbuf_column(t->iconview, 1);
    g_signal_connect(t->iconview, "selection-changed",
            G_CALLBACK(template_update_window), t->template_ok);
    return t;
}

void template_update_window(GdkEvent* event, void* button) {
    L_F_DEBUG;
    gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
}

const gchar* template_get(GuTemplate* templ) {
    L_F_DEBUG;
    const gchar* templates[] = { template_article, template_book,
                                 template_letter, template_report };
    GList* selection = gtk_icon_view_get_selected_items(templ->iconview);
    return templates[atoi(gtk_tree_path_to_string(selection->data))];
}
