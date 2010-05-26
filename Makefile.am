# libvbucket
# Copyright (C) 2010 NorthScale, Inc
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.
ACLOCAL_AMFLAGS = -I m4 --force

bin_PROGRAMS =
pkginclude_HEADERS = \
                     include/libvbucket/vbucket.h \
                     include/libvbucket/visibility.h

lib_LTLIBRARIES = libvbucket.la

check_PROGRAMS= testapp
TESTS = ${check_PROGRAMS}

# Test application to test stuff from C
testapp_SOURCES = tests/testapp.c
testapp_DEPENDENCIES= libvbucket.la
testapp_LDADD= libvbucket.la ${LIBHASHKIT}

libvbucket_la_SOURCES= \
                        src/cJSON.c \
                        src/cJSON.h \
                        src/vbucket.c
libvbucket_la_LDFLAGS=
libvbucket_la_LIBADD= ${LIBHASHKIT}


test: check-TESTS

