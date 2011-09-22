/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

#include "cJSON.h"
#include "hash.h"
#include <libvbucket/vbucket.h>

#define MAX_CONFIG_SIZE 100 * 1048576
#define MAX_BUCKETS 65536
#define MAX_REPLICAS 4
#define MAX_AUTORITY_SIZE 100
#define STRINGIFY(X) #X

struct server_st {
    char *authority;        /* host:port */
    char *couchdb_api_base;
};

struct vbucket_st {
    int servers[MAX_REPLICAS + 1];
};

struct continuum_item_st {
    uint32_t index;     /* server index */
    uint32_t point;     /* point on the ketama continuum */
};

struct vbucket_config_st {
    VBUCKET_DISTRIBUTION_TYPE distribution;
    int num_vbuckets;
    int mask;
    int num_servers;
    int num_replicas;
    char *user;
    char *password;
    int num_continuum;                      /* count of continuum points */
    struct continuum_item_st *continuum;    /* ketama continuum */
    struct server_st *servers;
    struct vbucket_st *fvbuckets;
    struct vbucket_st *vbuckets;
};

static char *errstr = NULL;

const char *vbucket_get_error() {
    return errstr;
}

static int continuum_item_cmp(const void *t1, const void *t2)
{
    const struct continuum_item_st *ct1 = t1, *ct2 = t2;

    if (ct1->point == ct2->point) {
        return 0;
    } else if (ct1->point > ct2->point) {
        return 1;
    } else {
        return -1;
    }
}

static void update_ketama_continuum(VBUCKET_CONFIG_HANDLE vb)
{
    char host[MAX_AUTORITY_SIZE+10] = "";
    int nhost;
    int pp, hh, ss, nn;
    unsigned char digest[16];
    struct continuum_item_st *new_continuum, *old_continuum;

    new_continuum = calloc(160 * vb->num_servers,
                           sizeof(struct continuum_item_st));

    /* 40 hashes, 4 numbers per hash = 160 points per server */
    for (ss = 0, pp = 0; ss < vb->num_servers; ++ss) {
        /* we can add more points to server which have more memory */
        for (hh = 0; hh < 40; ++hh) {
            nhost = snprintf(host, MAX_AUTORITY_SIZE+10, "%s-%u",
                             vb->servers[ss].authority, hh);
            hash_md5(host, nhost, digest);
            for (nn = 0; nn < 4; ++nn, ++pp) {
                new_continuum[pp].index = ss;
                new_continuum[pp].point = ((uint32_t) (digest[3 + nn * 4] & 0xFF) << 24)
                                        | ((uint32_t) (digest[2 + nn * 4] & 0xFF) << 16)
                                        | ((uint32_t) (digest[1 + nn * 4] & 0xFF) << 8)
                                        | (digest[0 + nn * 4] & 0xFF);
            }
        }
    }

    qsort(new_continuum, pp, sizeof(struct continuum_item_st), continuum_item_cmp);

    old_continuum = vb->continuum;
    vb->continuum = new_continuum;
    vb->num_continuum = pp;
    if (old_continuum) {
        free(old_continuum);
    }
}

void vbucket_config_destroy(VBUCKET_CONFIG_HANDLE vb) {
    int i;
    for (i = 0; i < vb->num_servers; ++i) {
        free(vb->servers[i].authority);
        free(vb->servers[i].couchdb_api_base);
    }
    free(vb->servers);
    free(vb->user);
    free(vb->password);
    free(vb->fvbuckets);
    memset(vb, 0xff, sizeof(vb));
    free(vb);
}

static int populate_servers(struct vbucket_config_st *vb, cJSON *c) {
    int i;

    vb->servers = calloc(vb->num_servers, sizeof(struct server_st));
    if (vb->servers == NULL) {
        vbucket_config_destroy(vb);
        errstr = "Failed to allocate servers array";
        return -1;
    }
    for (i = 0; i < vb->num_servers; ++i) {
        char *server;
        cJSON *jServer = cJSON_GetArrayItem(c, i);
        if (jServer == NULL || jServer->type != cJSON_String) {
            errstr = "Expected array of strings for serverList";
            return -1;
        }
        server = strdup(jServer->valuestring);
        if (server == NULL) {
            errstr = "Failed to allocate storage for server string";
            return -1;
        }
        vb->servers[i].authority = server;
    }
    return 0;
}

static int get_node_authority(cJSON *node, char *buf, size_t nbuf) {
    cJSON *json;
    char *hostname = NULL, *colon = NULL;
    int port = -1;

    json = cJSON_GetObjectItem(node, "hostname");
    if (json == NULL || json->type != cJSON_String) {
        errstr = "Expected string for node's hostname";
        return -1;
    }
    hostname = json->valuestring;
    json = cJSON_GetObjectItem(node, "ports");
    if (json == NULL || json->type != cJSON_Object) {
        errstr = "Expected json object for node's ports";
        return -1;
    }
    json = cJSON_GetObjectItem(json, "direct");
    if (json == NULL || json->type != cJSON_Number) {
        errstr = "Expected number for node's direct port";
        return -1;
    }
    port = json->valueint;

    snprintf(buf, nbuf - 7, "%s", hostname);
    colon = strchr(buf, ':');
    if (!colon) {
        colon = buf + strlen(buf);
    }
    snprintf(colon, 7, ":%d", port);

    return 0;
}

static int lookup_server_struct(struct vbucket_config_st *vb, cJSON *c) {
    char *authority = NULL;
    int idx = -1, ii;

    authority = calloc(MAX_AUTORITY_SIZE, sizeof(char));
    if (authority == NULL) {
        errstr = "Failed to allocate storage for authority string";
        return -1;
    }
    if (get_node_authority(c, authority, MAX_AUTORITY_SIZE) < 0) {
        free(authority);
        return -1;
    }

    for (ii = 0; ii < vb->num_servers; ++ii) {
        if (strcmp(vb->servers[ii].authority, authority) == 0) {
            idx = ii;
            break;
        }
    }

    free(authority);
    return idx;
}

static int update_server_info(struct vbucket_config_st *vb, cJSON *config) {
    int idx, ii;
    cJSON *node, *json;

    for (ii = 0; ii < cJSON_GetArraySize(config); ++ii) {
        node = cJSON_GetArrayItem(config, ii);
        if (node) {
            if (node->type != cJSON_Object) {
                errstr = "Expected json object for nodes array item";
                return -1;
            }

            if ((idx = lookup_server_struct(vb, node)) >= 0) {
                json = cJSON_GetObjectItem(node, "couchApiBase");
                if (json != NULL) {
                    char *value = strdup(json->valuestring);
                    if (value == NULL) {
                        errstr = "Failed to allocate storage for couchApiBase string";
                        return -1;
                    }
                    vb->servers[idx].couchdb_api_base = value;
                }
            }
        }
    }
    return 0;
}

static int populate_buckets(struct vbucket_config_st *vb, cJSON *c, int is_forward)
{
    int i, j;
    struct vbucket_st *vb_map = NULL;

    if (is_forward) {
        if (!(vb->fvbuckets = vb_map = calloc(vb->num_vbuckets, sizeof(struct vbucket_st)))) {
            errstr = "Failed to allocate storage for forward vbucket map";
            return -1;
        }
    } else {
        if (!(vb->vbuckets = vb_map = calloc(vb->num_vbuckets, sizeof(struct vbucket_st)))) {
            errstr = "Failed to allocate storage for vbucket map";
            return -1;
        }
    }

    for (i = 0; i < vb->num_vbuckets; ++i) {
        cJSON *jBucket = cJSON_GetArrayItem(c, i);
        if (jBucket == NULL || jBucket->type != cJSON_Array ||
            cJSON_GetArraySize(jBucket) != vb->num_replicas + 1) {
            errstr = "Expected array of arrays each with numReplicas + 1 ints for vBucketMap";
            return -1;
        }
        for (j = 0; j < vb->num_replicas + 1; ++j) {
            cJSON *jServerId = cJSON_GetArrayItem(jBucket, j);
            if (jServerId == NULL || jServerId->type != cJSON_Number ||
                jServerId->valueint < -1 || jServerId->valueint >= vb->num_servers) {
                errstr = "Server ID must be >= -1 and < num_servers";
                return -1;
            }
            vb_map[i].servers[j] = jServerId->valueint;
        }
    }
    return 0;
}

static VBUCKET_CONFIG_HANDLE parse_vbucket_config(VBUCKET_CONFIG_HANDLE vb, cJSON *c)
{
    cJSON *json, *config;

    config = cJSON_GetObjectItem(c, "vBucketServerMap");
    if (config == NULL || config->type != cJSON_Object) {
        /* seems like config without envelop, try to parse it */
        config = c;
    }

    json = cJSON_GetObjectItem(config, "numReplicas");
    if (json == NULL || json->type != cJSON_Number ||
        json->valueint > MAX_REPLICAS) {
        errstr = "Expected number <= " STRINGIFY(MAX_REPLICAS) " for numReplicas";
        return NULL;
    }
    vb->num_replicas = json->valueint;

    json = cJSON_GetObjectItem(config, "serverList");
    if (json == NULL || json->type != cJSON_Array) {
        errstr = "Expected array for serverList";
        return NULL;
    }
    vb->num_servers = cJSON_GetArraySize(json);
    if (vb->num_servers == 0) {
        errstr = "Empty serverList";
        return NULL;
    }
    if (populate_servers(vb, json) != 0) {
        return NULL;
    }
    /* optionally update server info using envelop (couchdb_api_base etc.) */
    json = cJSON_GetObjectItem(c, "nodes");
    if (json) {
        if (json->type != cJSON_Array) {
            errstr = "Expected array for nodes";
            return NULL;
        }
        if (update_server_info(vb, json) != 0) {
            return NULL;
        }
    }

    json = cJSON_GetObjectItem(config, "vBucketMap");
    if (json == NULL || json->type != cJSON_Array) {
        errstr = "Expected array for vBucketMap";
        return NULL;
    }
    vb->num_vbuckets = cJSON_GetArraySize(json);
    if (vb->num_vbuckets == 0 || (vb->num_vbuckets & (vb->num_vbuckets - 1)) != 0) {
        errstr = "Number of buckets must be a power of two > 0 and <= " STRINGIFY(MAX_BUCKETS);
        return NULL;
    }
    vb->mask = vb->num_vbuckets - 1;
    if (populate_buckets(vb, json, 0) != 0) {
        return NULL;
    }

    /* vbucket forward map could possibly be null */
    json = cJSON_GetObjectItem(config, "vBucketMapForward");
    if (json) {
        if (json->type != cJSON_Array) {
            errstr = "Expected array for vBucketMapForward";
            return NULL;
        }
        if (populate_buckets(vb, json, 1) !=0) {
            return NULL;
        }
    }

    return vb;
}

static VBUCKET_CONFIG_HANDLE parse_ketama_config(VBUCKET_CONFIG_HANDLE vb, cJSON *config)
{
    cJSON *json, *node;
    char *buf;
    int ii;

    json = cJSON_GetObjectItem(config, "nodes");
    if (json == NULL || json->type != cJSON_Array) {
        errstr = "Expected array for nodes";
        return NULL;
    }

    vb->num_servers = cJSON_GetArraySize(json);
    if (vb->num_servers == 0) {
        errstr = "Empty serverList";
        return NULL;
    }
    vb->servers = calloc(vb->num_servers, sizeof(struct server_st));
    for (ii = 0; ii < vb->num_servers; ++ii) {
        node = cJSON_GetArrayItem(json, ii);
        if (node == NULL || node->type != cJSON_Object) {
            errstr = "Expected object for nodes array item";
            return NULL;
        }
        buf = calloc(MAX_AUTORITY_SIZE, sizeof(char));
        if (buf == NULL) {
            errstr = "Failed to allocate storage for node authority";
            return NULL;
        }
        if (get_node_authority(node, buf, MAX_AUTORITY_SIZE) < 0) {
            return NULL;
        }
        vb->servers[ii].authority = buf;
    }

    update_ketama_continuum(vb);
    return vb;
}

static VBUCKET_CONFIG_HANDLE parse_cjson(cJSON *config)
{
    cJSON *json;
    struct vbucket_config_st *vb;

    vb = calloc(1, sizeof(struct vbucket_config_st));
    if (vb == NULL) {
        errstr = "Failed to allocate vbucket config struct";
        return NULL;
    }

    /* set optional credentials */
    json = cJSON_GetObjectItem(config, "name");
    if (json != NULL && json->type == cJSON_String && strcmp(json->valuestring, "default") != 0) {
        vb->user = strdup(json->valuestring);
    }
    json = cJSON_GetObjectItem(config, "saslPassword");
    if (json != NULL && json->type == cJSON_String) {
        vb->password = strdup(json->valuestring);
    }

    /* by default it uses vbucket distribution to map keys to servers */
    vb->distribution = VBUCKET_DISTRIBUTION_VBUCKET;

    json = cJSON_GetObjectItem(config, "nodeLocator");
    if (json == NULL) {
        /* special case: it migth be config without envelope */
        if (parse_vbucket_config(vb, config) == NULL) {
            vbucket_config_destroy(vb);
            return NULL;
        }
    } else if (json->type == cJSON_String) {
        if (strcmp(json->valuestring, "vbucket") == 0) {
            vb->distribution = VBUCKET_DISTRIBUTION_VBUCKET;
            if (parse_vbucket_config(vb, config) == NULL) {
                vbucket_config_destroy(vb);
                return NULL;
            }
        } else if (strcmp(json->valuestring, "ketama") == 0) {
            vb->distribution = VBUCKET_DISTRIBUTION_KETAMA;
            if (parse_ketama_config(vb, config) == NULL) {
                vbucket_config_destroy(vb);
                return NULL;
            }
        }
    } else {
        errstr = "Expected string for nodeLocator";
        vbucket_config_destroy(vb);
        return NULL;
    }

    return vb;
}

VBUCKET_CONFIG_HANDLE vbucket_config_parse_string(const char *data) {
    VBUCKET_CONFIG_HANDLE vb;
    cJSON *c = cJSON_Parse(data);
    if (c == NULL) {
        errstr = "Failed to parse data";
        return NULL;
    }

    vb = parse_cjson(c);

    cJSON_Delete(c);
    return vb;
}

VBUCKET_CONFIG_HANDLE vbucket_config_parse_file(const char *filename)
{
    long size;
    char *data;
    size_t nread;
    VBUCKET_CONFIG_HANDLE h;

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        errstr = "Unable to open file";
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size > MAX_CONFIG_SIZE) {
        fclose(f);
        errstr = "File too large";
        return NULL;
    }
    data = calloc(size+1, sizeof(char));
    if (data == NULL) {
        errstr = "Unable to allocate buffer to read file";
        return NULL;
    }
    nread = fread(data, sizeof(char), size+1, f);
    fclose(f);
    if (nread != (size_t)size) {
        free(data);
        errstr = "Failed to read entire file";
        return NULL;
    }
    h = vbucket_config_parse_string(data);
    free(data);
    return h;
}

int vbucket_map(VBUCKET_CONFIG_HANDLE vb, const void *key, size_t nkey,
                int *vbucket_id, int *server_idx)
{
    uint32_t digest, mid, prev;
    struct continuum_item_st *beginp, *endp, *midp, *highp, *lowp;

    if (vb->distribution == VBUCKET_DISTRIBUTION_KETAMA) {
        assert(vb->continuum);
        if (vbucket_id) {
            *vbucket_id = 0;
        }
        digest = hash_ketama(key, nkey);
        beginp = lowp = vb->continuum;
        endp = highp = vb->continuum + vb->num_continuum;

        /* divide and conquer array search to find server with next biggest
         * point after what this key hashes to */
        while (1)
        {
            /* pick the middle point */
            midp = lowp + (highp - lowp) / 2;

            if (midp == endp) {
                /* if at the end, roll back to zeroth */
                *server_idx = beginp->index;
                break;
            }

            mid = midp->point;
            prev = (midp == beginp) ? 0 : (midp-1)->point;

            if (digest <= mid && digest > prev) {
                /* we found nearest server */
                *server_idx = midp->index;
                break;
            }

            /* adjust the limits */
            if (mid < digest) {
                lowp = midp + 1;
            } else {
                highp = midp - 1;
            }

            if (lowp > highp) {
                *server_idx = beginp->index;
                break;
            }
        }
    } else {
        *vbucket_id = vbucket_get_vbucket_by_key(vb, key, nkey);
        *server_idx = vbucket_get_master(vb, *vbucket_id);
    }
    return 0;
}


int vbucket_config_get_num_replicas(VBUCKET_CONFIG_HANDLE vb) {
    return vb->num_replicas;
}

int vbucket_config_get_num_vbuckets(VBUCKET_CONFIG_HANDLE vb) {
    return vb->num_vbuckets;
}

int vbucket_config_get_num_servers(VBUCKET_CONFIG_HANDLE vb) {
    return vb->num_servers;
}

const char *vbucket_config_get_couch_api_base(VBUCKET_CONFIG_HANDLE vb, int i) {
    return vb->servers[i].couchdb_api_base;
}

VBUCKET_DISTRIBUTION_TYPE vbucket_config_get_distribution_type(VBUCKET_CONFIG_HANDLE vb) {
    return vb->distribution;
}

const char *vbucket_config_get_server(VBUCKET_CONFIG_HANDLE vb, int i) {
    return vb->servers[i].authority;
}

const char *vbucket_config_get_user(VBUCKET_CONFIG_HANDLE vb) {
    return vb->user;
}

const char *vbucket_config_get_password(VBUCKET_CONFIG_HANDLE vb) {
    return vb->password;
}

int vbucket_get_vbucket_by_key(VBUCKET_CONFIG_HANDLE vb, const void *key, size_t nkey) {
    /* call crc32 directly here it could be changed to some more general
     * function when vbucket distribution will support multiple hashing
     * algorithms */
    uint32_t digest = hash_crc32(key, nkey);
    return digest & vb->mask;
}

int vbucket_get_master(VBUCKET_CONFIG_HANDLE vb, int vbucket) {
    return vb->vbuckets[vbucket].servers[0];
}

int vbucket_get_replica(VBUCKET_CONFIG_HANDLE vb, int vbucket, int i) {
    return vb->vbuckets[vbucket].servers[i+1];
}

int vbucket_found_incorrect_master(VBUCKET_CONFIG_HANDLE vb, int vbucket,
                                   int wrongserver) {
    int mappedServer = vb->vbuckets[vbucket].servers[0];
    int rv = mappedServer;
    /*
     * if a forward table exists, then return the vbucket id from the forward table
     * and update that information in the current table. We also need to Update the
     * replica information for that vbucket
     */
    if (vb->fvbuckets) {
        int i = 0;
        rv = vb->vbuckets[vbucket].servers[0] = vb->fvbuckets[vbucket].servers[0];
        for (i = 0; i < vb->num_replicas; i++) {
            vb->vbuckets[vbucket].servers[i+1] = vb->fvbuckets[vbucket].servers[i+1];
        }
    } else if (mappedServer == wrongserver) {
        rv = (rv + 1) % vb->num_servers;
        vb->vbuckets[vbucket].servers[0] = rv;
    }

    return rv;
}

static void compute_vb_list_diff(VBUCKET_CONFIG_HANDLE from,
                                 VBUCKET_CONFIG_HANDLE to,
                                 char **out) {
    int offset = 0;
    int i, j;
    for (i = 0; i < to->num_servers; i++) {
        bool found = false;
        const char *sn = vbucket_config_get_server(to, i);
        for (j = 0; !found && j < from->num_servers; j++) {
            const char *sn2 = vbucket_config_get_server(from, j);
            found |= (strcmp(sn2, sn) == 0);
        }
        if (!found) {
            out[offset] = strdup(sn);
            assert(out[offset]);
            ++offset;
        }
    }
}

VBUCKET_CONFIG_DIFF* vbucket_compare(VBUCKET_CONFIG_HANDLE from,
                                     VBUCKET_CONFIG_HANDLE to) {
    VBUCKET_CONFIG_DIFF *rv = calloc(1, sizeof(VBUCKET_CONFIG_DIFF));
    int num_servers = (from->num_servers > to->num_servers
                       ? from->num_servers : to->num_servers) + 1;
    assert(rv);
    rv->servers_added = calloc(num_servers, sizeof(char*));
    rv->servers_removed = calloc(num_servers, sizeof(char*));

    /* Compute the added and removed servers */
    compute_vb_list_diff(from, to, rv->servers_added);
    compute_vb_list_diff(to, from, rv->servers_removed);

    /* Verify the servers are equal in their positions */
    if (to->num_servers == from->num_servers) {
        int i;
        rv->sequence_changed = false;
        for (i = 0; i < from->num_servers; i++) {
            rv->sequence_changed |= (0 != strcmp(vbucket_config_get_server(from, i),
                                                 vbucket_config_get_server(to, i)));

        }
    } else {
        /* Just say yes */
        rv->sequence_changed = true;
    }

    /* Consider the sequence changed if the auth credentials changed */
    if (from->user != NULL && to->user != NULL) {
        rv->sequence_changed |= (strcmp(from->user, to->user) != 0);
    } else {
        rv->sequence_changed |= ((from->user != NULL) ^ (to->user != NULL));
    }

    if (from->password != NULL && to->password != NULL) {
        rv->sequence_changed |= (strcmp(from->password, to->password) != 0);
    } else {
        rv->sequence_changed |= ((from->password != NULL) ^ (to->password != NULL));
    }

    /* Count the number of vbucket differences */
    if (to->num_vbuckets == from->num_vbuckets) {
        int i;
        for (i = 0; i < to->num_vbuckets; i++) {
            rv->n_vb_changes += (vbucket_get_master(from, i)
                                 == vbucket_get_master(to, i)) ? 0 : 1;
        }
    } else {
        rv->n_vb_changes = -1;
    }

    return rv;
}

static void free_array_helper(char **l) {
    int i;
    for (i = 0; l[i]; i++) {
        free(l[i]);
    }
    free(l);
}

void vbucket_free_diff(VBUCKET_CONFIG_DIFF *diff) {
    assert(diff);
    free_array_helper(diff->servers_added);
    free_array_helper(diff->servers_removed);
    free(diff);
}
