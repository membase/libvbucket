# libvbucket
# Copyright (C) 2010 NorthScale, Inc
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.
ACLOCAL_AMFLAGS = -I m4 --force

bin_PROGRAMS = vbuckettool vbucketkeygen
EXTRA_DIST = docs Doxyfile LICENSE README.markdown src/rfc1321

pkginclude_HEADERS = \
                     include/libvbucket/vbucket.h \
                     include/libvbucket/visibility.h

man_MANS =
if BUILD_DOCS
man_MANS += libvbucket.3lib \
            libvbucket.4 \
            vbucket_config_destroy.3vbucket \
            vbucket_config_get_num_replicas.3vbucket \
            vbucket_config_get_num_servers.3vbucket \
            vbucket_config_get_num_vbuckets.3vbucket \
            vbucket_config_get_server.3vbucket \
            vbucket_config_parse_file.3vbucket \
            vbucket_config_parse_string.3vbucket \
            vbucket_get_error.3vbucket \
            vbucket_get_master.3vbucket \
            vbucket_get_replica.3vbucket \
            vbucket_get_vbucket_by_key.3vbucket
endif

CLEANFILES= ${man_MANS}

libvbucket.3lib: docs/libvbucket.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

libvbucket.4: docs/fileformat.pod
	${POD2MAN} -c "$*" -r "" -s 4 $< $@

vbucket_config_parse_file.3vbucket: docs/allocation.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_config_parse_string.3vbucket: docs/allocation.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_config_destroy.3vbucket: docs/allocation.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_get_error.3vbucket: docs/misc.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_config_get_num_replicas.3vbucket: docs/config.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_config_get_num_vbuckets.3vbucket: docs/config.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_config_get_num_servers.3vbucket: docs/config.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_config_get_server.3vbucket: docs/config.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_get_vbucket_by_key.3vbucket: docs/vbucket.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_get_master.3vbucket: docs/vbucket.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

vbucket_get_replica.3vbucket: docs/vbucket.pod
	${POD2MAN} -c "$*" -r "" -s 3lib $< $@

noinst_LTLIBRARIES = libketama.la

lib_LTLIBRARIES = libvbucket.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libvbucket.pc

check_PROGRAMS= testapp regression testketama
TESTS = ${check_PROGRAMS}

# Test application to test stuff from C
testapp_SOURCES = tests/testapp.c
testapp_DEPENDENCIES= libvbucket.la
testapp_LDADD= libvbucket.la

regression_SOURCES = tests/regression.c
regression_DEPENDENCIES= libvbucket.la
regression_LDADD= libvbucket.la

testketama_SOURCES = tests/testketama.c
testketama_DEPENDENCIES= libvbucket.la libketama.la
testketama_LDADD= libvbucket.la libketama.la

libketama_la_SOURCES = src/ketama.c
libketama_la_CFLAGS =  $(AM_CFLAGS) ${NO_WERROR}

libvbucket_la_SOURCES= \
                        src/crc32.c \
                        src/hash.h \
                        src/cJSON.c \
                        src/cJSON.h \
                        src/vbucket.c

#
# To ensure that we don't break applications in mysterious ways we need
# to version our library. For a detailed description, check out:
# http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
#
current=1
revision=0
age=0
libvbucket_la_LDFLAGS= -version-info $(current):$(revision):$(age) -no-undefined
libvbucket_la_LIBADD = libketama.la
libvbucket_la_DEPENDENCIES = libketama.la

vbuckettool_SOURCES = src/vbuckettool.c
vbuckettool_DEPENDENCIES= libvbucket.la
vbuckettool_LDADD= libvbucket.la

vbucketkeygen_SOURCES = src/vbucketkeygen.c
vbucketkeygen_DEPENDENCIES= libvbucket.la
vbucketkeygen_LDADD= libvbucket.la

test: check-TESTS

