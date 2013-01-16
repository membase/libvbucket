# libvbucket
# Copyright (C) 2010, 2011 Couchbase, Inc
# All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
AC_PREREQ(2.59)
m4_include([m4/version.m4])
AC_INIT(libvbucket, VERSION_NUMBER, support@couchbase.com)
AC_CONFIG_SRCDIR([src/vbucket.c])
AC_CONFIG_AUX_DIR(config)
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE(subdir-objects)
AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# the malloc tests seems to be broken for cross compilation.. ignore them
ac_cv_func_malloc_0_nonnull=yes
ac_cv_func_realloc_0_nonnull=yes

COUCHBASE_GENERIC_COMPILER(c89)

AC_SEARCH_LIBS(floor, m)
AC_SEARCH_LIBS(pow, m)
AC_SEARCH_LIBS(fabs, m)

AC_ARG_WITH([docs],
  [AS_HELP_STRING([--with-docs],
    [Generate documentation (yes|no) @<:@default=yes@:>@])],
  [with_docs=$withval],
  [with_docs=yes])

AC_PATH_PROG(POD2MAN, pod2man, missing)
AS_IF([test "$with_docs" = "yes" -a "$POD2MAN" = "missing" ],
      [ AC_MSG_ERROR([pod2man is required for building docs])])
AM_CONDITIONAL(BUILD_DOCS, test "$with_docs" = "yes")

AC_CHECK_HEADERS_ONCE([stdint.h])
AS_IF([test "x$ac_cv_header_stdint_h" != "xyes"],
      [AC_MSG_ERROR(Failed to locate stdint.h)])

#
# To ensure that we don't break applications in mysterious ways we need
# to version our library. For a detailed description, check out:
# http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
#
LIBVBUCKET_API_CURRENT=2
LIBVBUCKET_API_REVISION=1
LIBVBUCKET_API_AGE=1
AC_SUBST(LIBVBUCKET_API_CURRENT)
AC_SUBST(LIBVBUCKET_API_REVISION)
AC_SUBST(LIBVBUCKET_API_AGE)
dnl ----------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile libvbucket.pc)
AC_OUTPUT
AS_IF(test "$with_docs" = "no",
  [
    echo "*****"
    echo "*"
    echo "*  WARNING: You are not generating any documentation."
    echo "*           Please don't ship libvbucket to an end user"
    echo "*           without documentation..."
    echo "*"
    echo "*****"
  ])
