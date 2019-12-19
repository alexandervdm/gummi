#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.65)
AC_INIT([Gummi], [0.8.0], [alexander@localhost], [gummi], [https://alexandervdm.github.io/gummi])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

GETTEXT_PACKAGE=gummi
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])
AC_SUBST(GETTEXT_PACKAGE)

# ls po/*.po | sort | sed 's/po\///g;s/.po//g'| sed ':a;N;s/\n/ /g;ta'
ALL_LINGUAS="ar ca cs da de el es fr hu it nl pl pt pt_BR ro ru sv zh_CN zh_TW"

AC_CONFIG_SRCDIR([src/environment.h])
AC_CONFIG_HEADERS([config.h])
AH_VERBATIM([USE_SYNCTEX1], [#undef USE_SYNCTEX1])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C_O
IT_PROG_INTLTOOL
AM_PROG_CC_C_O

# Checks for libraries.
AM_PATH_GLIB_2_0([2.20.0], [
	 GUI_CFLAGS="$GUI_CFLAGS $GLIB_CFLAGS"
	 GUI_LIBS="$GUI_LIBS $GLIB_LIBS"
	], [AC_MSG_ERROR([You need Glib >= 2.20.0 to build $PACKAGE])])

PKG_CHECK_MODULES(gthread, [gthread-2.0],,
	[AC_MSG_ERROR([You need ghread-2.0 to build $PACKAGE])])
GUI_CFLAGS="$GUI_CFLAGS $gthread_CFLAGS"
GUI_LIBS="$GUI_LIBS $gthread_LIBS"

AM_PATH_GTK_3_0([3.20.0], [
	 GUI_CFLAGS="$GUI_CFLAGS $GTK_CFLAGS"
	 GUI_LIBS="$GUI_LIBS $GTK_LIBS"
	], [AC_MSG_ERROR([You need Gtk+ >= 3.20.0 to build $PACKAGE])])

PKG_CHECK_MODULES(gtksourceview, [gtksourceview-3.0 >= 3.4.0],,
	[AC_MSG_ERROR([You need gtksourceview3 >= 3.4.0 to build $PACKAGE])])
GUI_CFLAGS="$GUI_CFLAGS $gtksourceview_CFLAGS"
GUI_LIBS="$GUI_LIBS $gtksourceview_LIBS"

PKG_CHECK_MODULES(poppler, [poppler-glib],,
	[AC_MSG_ERROR([You need Poppler to build $PACKAGE])])
GUI_CFLAGS="$GUI_CFLAGS $poppler_CFLAGS"
GUI_LIBS="$GUI_LIBS $poppler_LIBS"


PKG_CHECK_MODULES(gtkspell3, [gtkspell3-3.0],,
    [AC_MSG_ERROR([You need GtkSpell3 to build $PACKAGE])])
GUI_CFLAGS="$GUI_CFLAGS $gtkspell3_CFLAGS"
GUI_LIBS="$GUI_LIBS $gtkspell3_LIBS"

PKG_CHECK_MODULES(synctex, [synctex >= 1.16],,
	[AC_MSG_ERROR([You need synctex to build $PACKAGE])])
GUI_CFLAGS="$GUI_CFLAGS $synctex_CFLAGS"
GUI_LIBS="$GUI_LIBS $synctex_LIBS"

# Set USE_SYNCTEX1 for synctex (< 2.00) compatibility
if $PKG_CONFIG --max-version 1.99 synctex; then
  AC_DEFINE(USE_SYNCTEX1)
fi
AC_SUBST(USE_SYNCTEX1)

# Add explicit linking flags
GUI_LIBS="$GUI_LIBS -pthread -lm -lz"
AC_CHECK_LIB(rt, clock_gettime, [
    GUI_LIBS="$GUI_LIBS -lrt"
])

# Checks for header files.
AC_CHECK_HEADERS([libintl.h locale.h netdb.h stdlib.h string.h sys/socket.h sys/time.h unistd.h fcntl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname memset setlocale socket strstr])

AC_SUBST(GUI_LIBS)
AC_SUBST(GUI_CFLAGS)
AC_CONFIG_FILES([Makefile
		 src/Makefile
		 data/Makefile
		 data/ui/Makefile
		 data/icons/Makefile
		 data/snippets/Makefile
		 data/misc/Makefile
		 lib/Makefile
		 po/Makefile.in
		 data/misc/$PACKAGE.desktop
		 ])
AC_OUTPUT

echo
echo gummi...................................... : Version $VERSION
echo
echo The binary will be installed in $prefix/bin
echo 

echo configure complete, now type \'make\'.
echo
