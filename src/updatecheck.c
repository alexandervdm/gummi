/**
 * @file   updatecheck.c
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

#include "updatecheck.h"

#include <stdio.h>
#include <string.h>

#ifndef WIN32
#   include <sys/socket.h>
#   include <sys/time.h>
#   include <netdb.h>
#   include <unistd.h>
#endif

#include <glib.h>

#include "environment.h"
#include "utils.h"

#ifdef WIN32
/* TODO: use Winsock for WIN32 */
#else
gboolean updatecheck(GtkWindow* parent) {
    GtkWidget* dialog;
    struct sockaddr_in servaddr;
    struct hostent *hp;
    gint sock_fd = 0, i = 0;
    struct timeval timeout;
    gchar data[BUFSIZ] = { 0 };
    const gchar* avail_version;
    const gchar* request = "GET /redmine/projects/gummi/repository/raw/"
        "trunk/dev/latest HTTP/1.1\r\n"
        "User-Agent: Gummi\r\n"
        "Host: dev.midnightcoding.org\r\n"
        "\r\n";

    if (-1 == (sock_fd = socket(AF_INET, SOCK_STREAM, 0))) {
        slog(L_ERROR, "socket() error\n");
        return FALSE;
    }

    /* set timeout to prevent hanging */
    memset(&timeout, 0, sizeof(struct timeval));
    timeout.tv_sec = 5;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
            sizeof(struct timeval))) {
        slog(L_ERROR, "setsockopt() error\n");
        return FALSE;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    if (NULL == (hp = gethostbyname("dev.midnightcoding.org"))) {
        slog(L_ERROR, "gethostbyname() error\n");
        return FALSE;
    }

    memcpy((gchar*)&servaddr.sin_addr.s_addr, (gchar*)hp->h_addr, hp->h_length);
    servaddr.sin_port = htons(80);
    servaddr.sin_family = AF_INET;

    if (0 != connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
        slog(L_G_ERROR, "connect() error");
        return FALSE;
    }

    write(sock_fd, request, strlen(request));
    read(sock_fd, data, BUFSIZ);

    if (0 == strlen(data)) {
        slog(L_ERROR, "connection timeout\n");
        return FALSE;
    }

    /* get version string */
    for (i = strlen(data) -2; i >= 0 && data[i] != '\n'; --i);
    avail_version = data + i + 1;
    
    slog(L_INFO, "Currently installed: "PACKAGE_VERSION"\n");
    slog(L_INFO, "Currently available: %s", avail_version);

    dialog = gtk_message_dialog_new (parent, 
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        _("Currently installed:\n%s\n\nCurrently available:\n%s"),
        PACKAGE_VERSION, avail_version);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Update Check"));
    gtk_dialog_run(GTK_DIALOG(dialog));      
    gtk_widget_destroy(dialog);

    return TRUE;
}
#endif
