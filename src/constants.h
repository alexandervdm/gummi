/**
 * @file   constants.h
 * @brief  Constants used throughout the program
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


#ifdef WIN32
    /* brb, gonna go punch a wall */
    gchar *tmp_tmp = "C:\\gummitmp";
    g_mkdir_with_parents (tmp_tmp, DIR_PERMS);
    /* TODO: find out why Windows's env variables are still
             using goddamn 8.3 DOS format style and fix it. */
    #define C_TMPDIR tmp_tmp
    #define C_CMDSEP "&&"
    #define C_TEXSEC ""
#else
    #define C_TMPDIR g_get_tmp_dir()
    #define C_CMDSEP ";"
    #define C_TEXSEC "env openout_any=a"
#endif


