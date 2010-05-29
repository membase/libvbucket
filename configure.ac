# libvbucket
# Copyright (C) 2010 NorthScale, Inc
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.
AC_PREREQ(2.59)
m4_include([m4/version.m4])
AC_INIT(libvbucket, VERSION_NUMBER, trond@northscale.com)
AC_CONFIG_SRCDIR([src/vbucket.c])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_STATIC

PANDORA_CANONICAL_TARGET(less-warnings, warnings-always-on, force-gcc42)
PANDORA_REQUIRE_LIBHASHKIT
REQUIRE_POD2MAN

AH_TOP([
#ifndef CONFIG_H
#define CONFIG_H
/*
 * Copyright (C) 2010 NorthScale, Inc
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in this directory for full text.
 */
#ifdef _SYS_FEATURE_TESTS_H
#error "You should include config.h as your first include file"
#endif
])

if test "x${enable_dependency_tracking}" = "x"
then
  enable_dependency_tracking=yes
fi

AM_CPPFLAGS="$AM_CPPFLAGS -I\${top_srcdir}/include"

AS_IF([test "x$SUNCC" = "xyes"],
      [
        CFLAGS="-D_XOPEN_SOURCE=600 $CFLAGS"
        CPPFLAGS="-xldscope=hidden -mt $CPPFLAGS"
      ])

AH_BOTTOM([
#endif
])

dnl ----------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
echo "---"
echo "Configuration summary for $PACKAGE_NAME version $VERSION"
echo ""
echo "   * Installation prefix:       $prefix"
echo "   * System type:               $host_vendor-$host_os"
echo "   * Host CPU:                  $host_cpu"
echo "   * C Compiler:                $CC_VERSION"
echo "   * C++ Compiler:              $CXX_VERSION"
echo "   * Assertions enabled:        $ac_cv_assert"
echo "   * Debug enabled:             $with_debug"
echo "   * Warnings as failure:       $ac_cv_warnings_as_errors"
echo ""
echo "---"
