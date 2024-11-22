/**
 * @file   tectonic.c
 * @brief
 *
 * Copyright (C) 2009 Gummi Developers
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

#include "tectonic.h"

#include "configfile.h"
#include "constants.h"
#include "external.h"
#include "utils.h"

gboolean tec_detected = FALSE;

void tectonic_init (void) {
    if (external_exists (C_TECTONIC)) {
        slog (L_INFO, "Typesetter detected: Tectonic %s\n",
                       external_version (C_TECTONIC));
        tec_detected = TRUE;
    }
}

gboolean tectonic_active (void) {
    if (config_value_as_str_equals ("Compile", "typesetter", C_TECTONIC)) {
        return TRUE;
    }
    return FALSE;
}

gboolean tectonic_detected (void) {
    return tec_detected;
}

gchar* tectonic_get_command (const gchar* method, gchar* workfile) {
    const gchar* outdir = g_strdup_printf ("--outdir \"%s\"", C_TMPDIR);
    const gchar* flags = tectonic_get_flags (method);
    gchar* teccmd;

    teccmd = g_strdup_printf("tectonic %s %s \"%s\"", flags, outdir, workfile);

    return teccmd;
}

gchar* tectonic_get_flags (const gchar *method) {
    gchar *tecflags;
    if (STR_EQU (method, "texpdf")) {
        tecflags = g_strdup_printf("--outfmt pdf");
    }
    else {
        // TODO: figure out if tectonic can output whatever the other formats are
        tecflags = g_strdup_printf("");
    }

    if (config_get_boolean ("Compile", "synctex")) {
        tecflags = g_strconcat ("--synctex ", tecflags, NULL);
    }

    return tecflags;
}
