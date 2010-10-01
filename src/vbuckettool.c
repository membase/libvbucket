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

    VBUCKET_CONFIG_HANDLE vb = NULL;

    if (strcmp("-", argv[1]) == 0) {
        char buf[50000];
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

    int num_replicas = vbucket_config_get_num_replicas(vb);

    for (int i = 2; i < argc; i++) {
        char *key = argv[i];
        int v = vbucket_get_vbucket_by_key(vb, key, strlen(key));
        int m = vbucket_get_master(vb, v);
        const char *master = vbucket_config_get_server(vb, m);
        printf("key: %s vBucketId: %d master: %s", key, v, master);
        if (num_replicas > 0) {
            printf(" replicas:");
            for (int j = 0; j < num_replicas; j++) {
                int r = vbucket_get_replica(vb, v, j);
                if (r == -1) {
                    break;
                }

                const char *replica = vbucket_config_get_server(vb, r);
                if (replica != NULL) {
                    printf(" %s", replica);
                }
            }
        }
        printf("\n");
    }

    vbucket_config_destroy(vb);

    return 0;
}
