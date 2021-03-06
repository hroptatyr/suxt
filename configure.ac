dnl -------------------------------------------------------------------------
dnl Autoconf startup.
dnl -------------------------------------------------------------------------

AC_PREREQ([2.63])
AC_INIT([suxt], [0.1.0], [https://github.com/hroptatyr/suxt])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl -------------------------------------------------------------------------
dnl Local copyright notices.
dnl -------------------------------------------------------------------------

AC_COPYRIGHT([dnl
#### Configuration script for suxt
#### Copyright (C) 2010-2018 Sebastian Freundt

### Don't edit this script!
### This script was automatically generated by the `autoconf' program
### from the file `./configure.ac'.
### To rebuild it, execute the command
###     autoreconf
])

AM_INIT_AUTOMAKE([foreign parallel-tests dist-xz color-tests])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

## the build chain
AC_PROG_CC([icc cc gcc])
SXE_CHECK_CC([c11 c1x c99 gnu99])
SXE_CHECK_CFLAGS
AC_C_BIGENDIAN

## check if yuck is globally available
AX_CHECK_YUCK
AX_YUCK_SCMVER([version.mk])
AC_CONFIG_LINKS([GNUmakefile:GNUmakefile])

## also check for clitoris
AX_CHECK_CLITORIS

## libxml2 family
PKG_CHECK_MODULES([libxml2], [libxml-2.0 >= 2.0], [have_libxml2="yes"], [have_libxml2="no"])
PKG_CHECK_MODULES([libxslt], [libxslt], [have_libxslt="yes"], [have_libxslt="no"])
PKG_CHECK_MODULES([libexslt], [libexslt], [have_libexslt="yes"], [have_libexslt="no"])
AM_CONDITIONAL([HAVE_LIBXML2], [test "${have_libxml2}" = "yes"])
AM_CONDITIONAL([HAVE_LIBXSLT], [test "${have_libxslt}" = "yes"])
AM_CONDITIONAL([HAVE_LIBEXSLT], [test "${have_libexslt}" = "yes"])
## checks in more detail
save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${libxml2_CFLAGS} ${libxslt_CFLAGS} ${libexslt_CFLAGS}"
AC_CHECK_HEADERS([libxml/parser.h])
AC_CHECK_HEADERS([libxslt/xsltInternals.h])
AC_CHECK_HEADERS([libexslt/exslt.h])
CPPFLAGS="${save_CPPFLAGS}"

## expat
AM_CONDITIONAL([HAVE_EXPAT], [false])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([build-aux/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT

echo
echo
echo "Build summary"
echo "============="
echo
if test "${have_libxml2}" = "yes"; then
	echo "[[x]] xmlsplit"
else
	echo "[[ ]] xmlsplit"
fi
if test "${have_libxslt}" = "yes"; then
	echo "[[x]] xsltmultiproc with"
	if test "${have_libexslt}" = "yes"; then
		echo " -  [[x]] exslt extensions"
	else
		echo " -  [[ ]] exslt extensions"
	fi
else
	echo "[[ ]] xsltmultiproc"
fi
echo

## configure ends here
dnl configure.ac ends here
