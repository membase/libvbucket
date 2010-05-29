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

man_MANS = libvbucket.3lib \
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



lib_LTLIBRARIES = libvbucket.la

check_PROGRAMS= testapp
TESTS = ${check_PROGRAMS}

# Test application to test stuff from C
testapp_SOURCES = tests/testapp.c
testapp_DEPENDENCIES= libvbucket.la
testapp_LDADD= libvbucket.la ${LTLIBHASHKIT}

libvbucket_la_SOURCES= \
                        src/cJSON.c \
                        src/cJSON.h \
                        src/vbucket.c
libvbucket_la_LDFLAGS=
libvbucket_la_LIBADD= ${LTLIBHASHKIT}


test: check-TESTS

