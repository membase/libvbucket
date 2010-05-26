/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#ifndef VBUCKET_H
#define VBUCKET_H 1

#include <stddef.h>
#include "visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* VBUCKET_CONFIG_HANDLE;

/* Creation and destruction */
LIBVBUCKET_PUBLIC_API
VBUCKET_CONFIG_HANDLE vbucket_config_parse_file(const char *filename);
LIBVBUCKET_PUBLIC_API
VBUCKET_CONFIG_HANDLE vbucket_config_parse_string(const char *data);
LIBVBUCKET_PUBLIC_API
void vbucket_config_destroy(VBUCKET_CONFIG_HANDLE h);

/* Misc */
LIBVBUCKET_PUBLIC_API
const char *vbucket_get_error(void);

/* Config access */
LIBVBUCKET_PUBLIC_API
int vbucket_config_get_num_replicas(VBUCKET_CONFIG_HANDLE h);
LIBVBUCKET_PUBLIC_API
int vbucket_config_get_num_vbuckets(VBUCKET_CONFIG_HANDLE h);
LIBVBUCKET_PUBLIC_API
int vbucket_config_get_num_servers(VBUCKET_CONFIG_HANDLE h);
LIBVBUCKET_PUBLIC_API
const char *vbucket_config_get_server(VBUCKET_CONFIG_HANDLE h, int i);

/* VBucket access */
LIBVBUCKET_PUBLIC_API
int vbucket_get_vbucket_by_key(VBUCKET_CONFIG_HANDLE h, const void *key, size_t nkey);
LIBVBUCKET_PUBLIC_API
int vbucket_get_master(VBUCKET_CONFIG_HANDLE h, int id);
LIBVBUCKET_PUBLIC_API
int vbucket_get_replica(VBUCKET_CONFIG_HANDLE h, int id, int n);

#ifdef __cplusplus
}
#endif

#endif
