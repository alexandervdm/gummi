#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.65)
AC_INIT([Gummi], [0.5.2~svn515], [alexvandermey@gmail.com], [gummi], [http://gummi.midnightcoding.org/])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# ls po/*.po | sort | sed 's/po\///g;s/.po//g'| sed ':a;N;s/\n/ /g;ta'
GETTEXT_PACKAGE=gummi
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])
AC_SUBST(GETTEXT_PACKAGE)

ALL_LINGUAS="ca da fr el it nl pt_BR ro ru zh_TW"

AC_CONFIG_SRCDIR([src/environment.h])
AC_CONFIG_HEADERS([config.h])
AH_VERBATIM([USE_GTKSPELL], [#undef USE_GTKSPELL])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C_O
IT_PROG_INTLTOOL
AM_PROG_CC_C_O

# Checks for libraries.
AC_ARG_ENABLE([spellcheck],
	AS_HELP_STRING([--disable-spellcheck],[disable spellcheck support]),
	spellcheck=$enableval, spellcheck="yes")

if test "$spellcheck" = yes; then
    AC_DEFINE(USE_GTKSPELL)
fi
AC_SUBST(USE_GTKSPELL)

AM_PATH_GLIB_2_0([2.16.0], [
	 GUI_CFLAGS="$GUI_CFLAGS $GLIB_CFLAGS"
	 GUI_LIBS="$GUI_LIBS $GLIB_LIBS"
	], [AC_MSG_ERROR([You need Glib >= 2.16.0 to build $PACKAGE])]) 

AM_PATH_GTK_2_0([2.16.0], [
	 GUI_CFLAGS="$GUI_CFLAGS $GTK_CFLAGS"
	 GUI_LIBS="$GUI_LIBS $GTK_LIBS"
	], [AC_MSG_ERROR([You need Gtk+ >= 2.16.0 to build $PACKAGE])]) 

PKG_CHECK_MODULES(gtksourceview, [gtksourceview-2.0 >= 2.2.1],,
	[AC_MSG_ERROR([You need GtkSourceView2 >= 2.2.1 to build $PACKAGE])]) 
GUI_CFLAGS="$GUI_CFLAGS $gtksourceview_CFLAGS"
GUI_LIBS="$GUI_LIBS $gtksourceview_LIBS"

PKG_CHECK_MODULES(poppler, [poppler-glib],,
	[AC_MSG_ERROR([You need Poppler to build $PACKAGE])]) 
GUI_CFLAGS="$GUI_CFLAGS $poppler_CFLAGS"
GUI_LIBS="$GUI_LIBS $poppler_LIBS"

PKG_CHECK_MODULES(gtkspell, [gtkspell-2.0],,
	[AC_MSG_ERROR([You need GtkSpell  to build $PACKAGE])]) 
GUI_CFLAGS="$GUI_CFLAGS $gtkspell_CFLAGS"
GUI_LIBS="$GUI_LIBS $gtkspell_LIBS"

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
		 po/Makefile.in
		 data/$PACKAGE.desktop
		 ])
AC_OUTPUT

echo
echo gummi...................................... : Version $VERSION
echo
echo spellcheck support......................... : $spellcheck
echo
echo The binary will be installed in $prefix/bin
echo 

echo configure complete, now type \'make\'.
echo
