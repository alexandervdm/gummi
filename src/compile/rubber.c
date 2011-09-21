/**
 * @file   rubber.c
 * @brief  
 *
 * Copyright (C) 2010-2011 Gummi-Dev Team <alexvandermey@gmail.com>
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
#include "utils.h"

gboolean detected = FALSE;

void rubber_init (void) {
    
    if (utils_program_exists (C_RUBBER)) {
        // TODO: check if supported version
        slog (L_INFO, "Typesetter detected: %s\n", utils_get_version (C_RUBBER));
        detected = TRUE;
    }
}

gboolean rubber_active (void) {
    if (g_strcmp0 (config_get_value("typesetter"), C_RUBBER) == 0) {
        return TRUE;
    }
    return FALSE;
}

gboolean rubber_detected (void) {
    return detected;
}

/* base form : 
 * 
 * cd "/tmp"; env openout_any=a rubber -p -d -q --into="/tmp" "/tmp/gummi_4I2B2V"
 * 
 */







