/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libvbucket/vbucket.h>

int main(int argc, char **argv) {
    VBUCKET_CONFIG_HANDLE vb = NULL;
    int num_replicas;
    int i;

    if (argc < 3) {
        printf("vbuckettool mapfile key0 [key1 ... [keyN]]\n\n");
        printf("  The vbuckettool expects a vBucketServerMap JSON mapfile, and\n");
        printf("  will print the vBucketId and servers each key should live on.\n");
        printf("  You may use '-' instead for the filename to specify stdin.\n\n");
        printf("  Examples:\n");
        printf("    ./vbuckettool file.json some_key another_key\n\n");
        printf("    curl http://HOST:8091/pools/default/buckets/default | \\\n");
        printf("       ./vbuckettool - some_key another_key\n");
        exit(1);
    }

    if (strcmp("-", argv[1]) == 0) {
        char buf[500000];
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            fprintf(stderr, "ERROR: vbuckettool found no input on stdin\n");
            exit(1);
        }
        buf[sizeof(buf) - 1] = '\0';

        vb = vbucket_config_parse_string(buf);
    } else {
        vb = vbucket_config_parse_file(argv[1]);
    }

    if (vb == NULL) {
        fprintf(stderr, "ERROR: vbucket_config_parse_string error: %s\n", vbucket_get_error());
        exit(1);
    }

    num_replicas = vbucket_config_get_num_replicas(vb);

    for (i = 2; i < argc; i++) {
        char *key = argv[i];
        int v, m;
        const char *master, *couch_api_base;

        if (vbucket_map(vb, key, strlen(key), &v, &m) < -1) {
            fprintf(stderr, "ERROR: vbucket_map(..., \"%s\", ...) error: %s\n", key, vbucket_get_error());
            continue;
        }
        master = vbucket_config_get_server(vb, m);
        couch_api_base = vbucket_config_get_couch_api_base(vb, m);
        printf("key: %s master: %s", key, master);
        if (vbucket_config_get_distribution_type(vb) == VBUCKET_DISTRIBUTION_VBUCKET) {
            printf(" vBucketId: %d couchApiBase: %s", v, couch_api_base);
            if (num_replicas > 0) {
                int j;
                printf(" replicas:");
                for (j = 0; j < num_replicas; j++) {
                    int r = vbucket_get_replica(vb, v, j);
                    const char *replica;

                    if (r == -1) {
                        break;
                    }

                    replica = vbucket_config_get_server(vb, r);
                    if (replica != NULL) {
                        printf(" %s", replica);
                    }
                }
            }
        }
        printf("\n");
    }

    vbucket_config_destroy(vb);

    return 0;
}
