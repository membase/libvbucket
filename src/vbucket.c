/*
 *     Copyright 2010 NorthScale, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */



#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <libhashkit/hashkit.h>

#include "cJSON.h"
#include <libvbucket/vbucket.h>

#define ERRBUF_SIZE 1024
#define MAX_CONFIG_SIZE 100 * 1048576
#define MAX_BUCKETS 65536
#define MAX_REPLICAS 4
#define STRINGIFY(X) #X

struct vbucket_st {
    int servers[MAX_REPLICAS + 1];
};

struct vbucket_config_st {
    hashkit_hash_algorithm_t hk_algorithm;
    int num_vbuckets;
    int mask;
    int num_servers;
    int num_replicas;
    char **servers;
    struct vbucket_st vbuckets[];
};

static char *errstr = NULL;

const char *vbucket_get_error() {
    return errstr;
}

static hashkit_hash_algorithm_t lookup_hash_algorithm(const char *s) {
    static char *hashes[HASHKIT_HASH_MAX];
    hashes[HASHKIT_HASH_DEFAULT] = "default";
    hashes[HASHKIT_HASH_MD5] = "md5";
    hashes[HASHKIT_HASH_CRC] = "crc";
    hashes[HASHKIT_HASH_FNV1_64] = "fnv1_64";
    hashes[HASHKIT_HASH_FNV1A_64] = "fnv1a_64";
    hashes[HASHKIT_HASH_FNV1_32] = "fnv1_32";
    hashes[HASHKIT_HASH_FNV1A_32] = "fnv1a_32";
    hashes[HASHKIT_HASH_HSIEH] = "hsieh";
    hashes[HASHKIT_HASH_MURMUR] = "murmur";
    hashes[HASHKIT_HASH_JENKINS] = "jenkins";
    for (unsigned int i = 0; i < sizeof(hashes); ++i) {
        if (hashes[i] != NULL && strcasecmp(s, hashes[i]) == 0) {
            return i;
        }
    }
    return HASHKIT_HASH_MAX;
}

static struct vbucket_config_st *config_create(char *hash_algorithm,
                                               int num_servers,
                                               int num_vbuckets,
                                               int num_replicas) {
    hashkit_hash_algorithm_t ha = lookup_hash_algorithm(hash_algorithm);
    if (ha == HASHKIT_HASH_MAX) {
        errstr = "Bogus hash algorithm specified";
        return NULL;
    }

    struct vbucket_config_st *vb = calloc(sizeof(struct vbucket_config_st) +
                                          sizeof(struct vbucket_st) * num_vbuckets, 1);
    if (vb == NULL) {
        errstr = "Failed to allocate vbucket config struct";
        return NULL;
    }

    vb->hk_algorithm = ha;

    vb->servers = calloc(sizeof(char*), num_servers);
    if (vb->servers == NULL) {
        free(vb);
        errstr = "Failed to allocate servers array";
        return NULL;
    }
    vb->num_servers = num_servers;
    vb->num_vbuckets = num_vbuckets;
    vb->num_replicas = num_replicas;
    vb->mask = num_vbuckets - 1;
    return vb;
}

void vbucket_config_destroy(VBUCKET_CONFIG_HANDLE h) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    for (int i = 0; i < vb->num_servers; ++i) {
        free(vb->servers[i]);
    }
    free(vb->servers);
    free(vb);
}

static int populate_servers(struct vbucket_config_st *vb, cJSON *c) {
    for (int i = 0; i < vb->num_servers; ++i) {
        cJSON *jServer = cJSON_GetArrayItem(c, i);
        if (jServer == NULL || jServer->type != cJSON_String) {
            errstr = "Expected array of strings for serverList";
            return -1;
        }
        char *server = strdup(jServer->valuestring);
        if (server == NULL) {
            errstr = "Failed to allocate storage for server string";
            return -1;
        }
        vb->servers[i] = server;
    }
    return 0;
}

static int populate_buckets(struct vbucket_config_st *vb, cJSON *c) {
    for (int i = 0; i < vb->num_vbuckets; ++i) {
        cJSON *jBucket = cJSON_GetArrayItem(c, i);
        if (jBucket == NULL || jBucket->type != cJSON_Array ||
            cJSON_GetArraySize(jBucket) != vb->num_replicas + 1) {
            errstr = "Expected array of arrays each with numReplicas + 1 ints for vBucketMap";
            return -1;
        }
        for (int j = 0; j < vb->num_replicas + 1; ++j) {
            cJSON *jServerId = cJSON_GetArrayItem(jBucket, j);
            if (jServerId == NULL || jServerId->type != cJSON_Number ||
                jServerId->valueint < -1 || jServerId->valueint >= vb->num_servers) {
                errstr = "Server ID must be >= -1 and < num_servers";
                return -1;
            }
            vb->vbuckets[i].servers[j] = jServerId->valueint;
        }
    }
    return 0;
}

static VBUCKET_CONFIG_HANDLE parse_cjson(cJSON *c) {
    cJSON *jHashAlgorithm = cJSON_GetObjectItem(c, "hashAlgorithm");
    if (jHashAlgorithm == NULL || jHashAlgorithm->type != cJSON_String) {
        errstr = "Expected string for hashAlgorithm";
        return NULL;
    }
    char *hashAlgorithm = jHashAlgorithm->valuestring;

    cJSON *jNumReplicas = cJSON_GetObjectItem(c, "numReplicas");
    if (jNumReplicas == NULL || jNumReplicas->type != cJSON_Number ||
        jNumReplicas->valueint > MAX_REPLICAS) {
        errstr = "Expected number <= " STRINGIFY(MAX_REPLICAS) " for numReplicas";
        return NULL;
    }
    int numReplicas = jNumReplicas->valueint;

    cJSON *jServers = cJSON_GetObjectItem(c, "serverList");
    if (jServers == NULL || jServers->type != cJSON_Array) {
        errstr = "Expected array for serverList";
        return NULL;
    }

    int numServers = cJSON_GetArraySize(jServers);
    if (numServers == 0) {
        errstr = "Empty serverList";
        return NULL;
    }

    cJSON *jBuckets = cJSON_GetObjectItem(c, "vBucketMap");
    if (jBuckets == NULL || jBuckets->type != cJSON_Array) {
        errstr = "Expected array for vBucketMap";
        return NULL;
    }

    int numBuckets = cJSON_GetArraySize(jBuckets);
    if (numBuckets == 0 || (numBuckets & (numBuckets - 1)) != 0) {
        errstr = "Number of buckets must be a power of two > 0 and <= " STRINGIFY(MAX_BUCKETS);
        return NULL;
    }

    struct vbucket_config_st *vb = config_create(hashAlgorithm, numServers,
                                                 numBuckets, numReplicas);
    if (vb == NULL) {
        return NULL;
    }

    if (populate_servers(vb, jServers) != 0) {
        vbucket_config_destroy(vb);
        return NULL;
    }


    if (populate_buckets(vb, jBuckets) != 0) {
        vbucket_config_destroy(vb);
        return NULL;
    }
    return vb;
}

VBUCKET_CONFIG_HANDLE vbucket_config_parse_string(const char *data) {
    cJSON *c = cJSON_Parse(data);
    if (c == NULL) {
        return NULL;
    }

    VBUCKET_CONFIG_HANDLE vb = parse_cjson(c);

    cJSON_Delete(c);
    return vb;
}

VBUCKET_CONFIG_HANDLE vbucket_config_parse_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size > MAX_CONFIG_SIZE) {
        fclose(f);
        return NULL;
    }
    char *data = calloc(sizeof(char), size+1);
    if (data == NULL) {
        return NULL;
    }
    size_t nread = fread(data, sizeof(char), size+1, f);
    fclose(f);
    if (nread != (size_t)size) {
        free(data);
        return NULL;
    }
    VBUCKET_CONFIG_HANDLE h = vbucket_config_parse_string(data);
    free(data);
    return h;
}

int vbucket_config_get_num_replicas(VBUCKET_CONFIG_HANDLE h) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    return vb->num_replicas;
}

int vbucket_config_get_num_vbuckets(VBUCKET_CONFIG_HANDLE h) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    return vb->num_vbuckets;
}

int vbucket_config_get_num_servers(VBUCKET_CONFIG_HANDLE h) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    return vb->num_servers;
}

const char *vbucket_config_get_server(VBUCKET_CONFIG_HANDLE h, int i) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    return vb->servers[i];
}

int vbucket_get_vbucket_by_key(VBUCKET_CONFIG_HANDLE h, const void *key, size_t nkey) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    uint32_t digest = libhashkit_digest(key, nkey, vb->hk_algorithm);
    return digest & vb->mask;
}

int vbucket_get_master(VBUCKET_CONFIG_HANDLE h, int vbucket) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    return vb->vbuckets[vbucket].servers[0];
}

int vbucket_get_replica(VBUCKET_CONFIG_HANDLE h, int vbucket, int i) {
    struct vbucket_config_st *vb = (struct vbucket_config_st*)h;
    return vb->vbuckets[vbucket].servers[i+1];
}

