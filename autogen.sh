#! /bin/sh

# modified from the freetype2 project - thank you!
check_tool_version () {
  field=$2
  # assume output of "[TOOL] --version" is "toolname (GNU toolname foo bar) version"
  if test "$field"x = x; then
    field=3  # default to 3 for all GNU autotools, after filtering enclosed string
  fi
  version=`$1 --version | head -1 | sed 's/([^)]*)/()/g' | cut -d ' ' -f $field`
  # cut off patch from version format (major.minor.patch)
  echo `echo $version | cut -d"." -f1,2`
}

AM_VERSION=`check_tool_version automake`
AC_VERSION=

set -x

if [ "x${ACLOCAL_DIR}" != "x" ]; then
  ACLOCAL_ARG=-I ${ACLOCAL_DIR}
fi

${ACLOCAL:-aclocal-$AM_VERSION} ${ACLOCAL_ARG}
${AUTOHEADER:-autoheader$AC_VERSION}
AUTOMAKE=${AUTOMAKE:-automake-$AM_VERSION} libtoolize -c --automake
${AUTOMAKE:-automake-$AM_VERSION} --add-missing --copy --include-deps
${AUTOCONF:-autoconf$AC_VERSION}

# mkinstalldirs was not correctly installed in some cases.
cp -f /usr/share/automake-${AM_VERSION}/mkinstalldirs .

rm -rf autom4te.cache
