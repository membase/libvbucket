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
ACLOCAL_AMFLAGS = -I m4 --force

bin_PROGRAMS = vbuckettool vbucketkeygen
EXTRA_DIST = docs Doxyfile LICENSE README.markdown src/rfc1321 NMakefile

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

noinst_LTLIBRARIES = libketama.la libcrc32.la

lib_LTLIBRARIES = libvbucket.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libvbucket.pc

check_PROGRAMS= testapp regression testketama
TESTS = ${check_PROGRAMS}

# Test application to test stuff from C
testapp_SOURCES = tests/testapp.c tests/macros.h
testapp_CFLAGS = $(AM_CFLAGS) $(NO_WERROR)
testapp_DEPENDENCIES= libvbucket.la
testapp_LDADD= libvbucket.la

regression_SOURCES = tests/regression.c tests/macros.h
regression_CFLAGS = $(AM_CFLAGS) $(NO_WERROR)
regression_DEPENDENCIES= libvbucket.la
regression_LDADD= libvbucket.la

testketama_SOURCES = tests/testketama.c tests/macros.h
testketama_CFLAGS = $(AM_CFLAGS) $(NO_WERROR)
testketama_DEPENDENCIES= libvbucket.la libketama.la
testketama_LDADD= libvbucket.la libketama.la

# Configuration files needed by check target
EXTRA_DIST += tests/config

libketama_la_SOURCES = src/ketama.c
libketama_la_CFLAGS =  $(AM_CFLAGS) ${NO_WERROR}

libcrc32_la_SOURCES = src/crc32.c
libcrc32_la_CFLAGS = $(AM_CFLAGS) ${NO_WERROR}

libvbucket_la_SOURCES= \
                        src/hash.h \
                        src/cJSON.c \
                        src/cJSON.h \
                        src/vbucket.c

#
# To ensure that we don't break applications in mysterious ways we need
# to version our library. For a detailed description, check out:
# http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
#
libvbucket_la_LDFLAGS= -version-info $(LIBVBUCKET_API_CURRENT):$(LIBVBUCKET_API_REVISION):$(LIBVBUCKET_API_AGE) -no-undefined
libvbucket_la_LIBADD = libketama.la libcrc32.la
libvbucket_la_DEPENDENCIES = libketama.la libcrc32.la

vbuckettool_SOURCES = src/vbuckettool.c
vbuckettool_DEPENDENCIES= libvbucket.la
vbuckettool_LDADD= libvbucket.la

vbucketkeygen_SOURCES = src/vbucketkeygen.c
vbucketkeygen_DEPENDENCIES= libvbucket.la
vbucketkeygen_LDADD= libvbucket.la

test: check-TESTS

sinclude packaging/*/package.mk
