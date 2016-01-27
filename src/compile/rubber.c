/**
 * @file   rubber.c
 * @brief
 *
 * Copyright (C) 2010-2011 Gummi Developers
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

#include "rubber.h"

#include "configfile.h"
#include "constants.h"
#include "external.h"
#include "utils.h"

gboolean rub_detected = FALSE;

void rubber_init (void) {

    if (external_exists (C_RUBBER)) {
        // TODO: check if supported version
        slog (L_INFO, "Typesetter detected: Rubber %s\n",
                       external_version (C_RUBBER));
        rub_detected = TRUE;
    }
}

gboolean rubber_active (void) {
    if (STR_EQU (config_get_value("typesetter"), C_RUBBER)) {
        return TRUE;
    }
    return FALSE;
}

gboolean rubber_detected (void) {
    return rub_detected;
}

gchar* rubber_get_command (const gchar* method, gchar* workfile) {

    const gchar* outdir = g_strdup_printf ("--into=\"%s\"", C_TMPDIR);
    const gchar* flags = rubber_get_flags (method);
    gchar* rubcmd;

    rubcmd = g_strdup_printf("rubber %s %s \"%s\"", flags, outdir, workfile);

    return rubcmd;
}

gchar* rubber_get_flags (const gchar *method) {
    gchar *rubflags;
    if (STR_EQU (method, "texpdf")) {
        rubflags = g_strdup_printf("-d -q");
    }
    else {
        rubflags = g_strdup_printf("-p -d -q");
    }
    return rubflags;
}
