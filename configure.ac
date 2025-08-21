dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.62)
AC_INIT([cafe-calc], [2.0.0], [http://cafe-desktop.org])

AM_INIT_AUTOMAKE([1.9 foreign no-dist-gzip dist-xz check-news])
AC_CONFIG_HEADERS(config.h)
AM_MAINTAINER_MODE
CAFE_MAINTAINER_MODE_DEFINES
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC

GLIB_GSETTINGS
CAFE_COMPILE_WARNINGS

dnl ###########################################################################
dnl Dependencies
dnl ###########################################################################

GLIB_REQUIRED=2.50.0
GIO_REQUIRED=2.36.0
CTK_REQUIRED=3.22.0

PKG_CHECK_MODULES(CAFE_CALC, [
    ctk+-3.0 >= $CTK_REQUIRED
    glib-2.0 >= $GLIB_REQUIRED
    gio-2.0 >= $GIO_REQUIRED
    libxml-2.0
    gmodule-export-2.0
])

PKG_CHECK_MODULES(CAFE_CALC_CMD, [
    glib-2.0 >= $GLIB_REQUIRED
    gio-2.0 >= $GIO_REQUIRED
    libxml-2.0
])

GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_MKENUMS)

AC_CHECK_LIB(m, log)

dnl ###########################################################################
dnl Internationalization
dnl ###########################################################################
AM_GNU_GETTEXT_VERSION([0.19.8])
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.8])
AM_GNU_GETTEXT([external])
AC_SUBST(GETTEXT_PACKAGE, cafe-calc)

# **********************
# GLIB_COMPILE_RESOURCES
# **********************
AC_ARG_VAR([GLIB_COMPILE_RESOURCES],[glib-compile-resources bin])
AC_PATH_PROG([GLIB_COMPILE_RESOURCES],[glib-compile-resources],[])
if test -z "$GLIB_COMPILE_RESOURCES"; then
  AC_MSG_ERROR([glib-compile-resources not found])
fi

# *******
# XMLLINT
# *******
AC_ARG_VAR([XMLLINT],[xmllint bin])
AC_PATH_PROG([XMLLINT],[xmllint],[])
if test -z "$XMLLINT"; then
  AC_MSG_ERROR([xmllint not found])
fi

dnl ###########################################################################
dnl Documentation
dnl ###########################################################################

YELP_HELP_INIT

dnl ###########################################################################
dnl Files to generate
dnl ###########################################################################

AC_CONFIG_FILES([
Makefile
src/Makefile
po/Makefile.in
data/Makefile
help/Makefile
])

AC_OUTPUT
