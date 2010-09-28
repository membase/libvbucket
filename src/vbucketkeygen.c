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

size_t MAX_KEY_SIZE = 14;

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("vbucketkeygen mapfile <keys per vbucket> <keys to generate>\n\n");
        printf("  vbucketkeygen will output a list of keys that equally\n");
        printf("    distribute amongst every vbucket.\n\n");
        printf("  vbucketkeygen expects a vBucketServerMap JSON mapfile, and\n");
        printf("  will print the keyname and vBucketId.\n");
        printf("  You may use '-' instead for the filename to specify stdin.\n\n");
        printf("  Examples:\n");
        printf("    ./vbucketkeygen file.json 10 10000\n\n");
        printf("    curl http://HOST:8091/pools/default/buckets/default | \\\n");
        printf("       ./vbucketkeygen - 5 10000\n");
        exit(1);
    }

    VBUCKET_CONFIG_HANDLE vb = NULL;

    if (strcmp("-", argv[1]) == 0) {
        char buf[50000];
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            fprintf(stderr, "ERROR: vbucketkeygen found no input on stdin\n");
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

    int rval = 0;
    int num_keys_per_vbucket = atoi(argv[2]);
    int num_keys_to_generate = atoi(argv[3]);
    int num_vbuckets = vbucket_config_get_num_vbuckets(vb);
    char *** keys;

    /* allocate memory and set each key to null since strdup will allocate that */
    keys = malloc(sizeof(char***) * num_vbuckets);
    for (int i = 0; i < num_vbuckets; i++) {
        keys[i] = malloc(sizeof(char**) * num_keys_per_vbucket);
    }
    for (int i = 0; i < num_vbuckets; i++) {
        for (int j = 0 ; j < num_keys_per_vbucket ; j++) {
            keys[i][j] = 0;
        }
    }

    /* generate keys and copy them to the keys structure */
    char * key = malloc(sizeof(char) * (MAX_KEY_SIZE+1));
    for (int i = 0; i < num_keys_to_generate; i++) {
        snprintf(key, MAX_KEY_SIZE + 1, "key_%010d", i);
        int v = vbucket_get_vbucket_by_key(vb, key, strlen(key));
        for (int k = 0; k < num_keys_per_vbucket; k++) {
            if (keys[v][k] == 0) {
                keys[v][k] = strdup(key);
                break;
            }
        }
    }

    /* print out <key> <vbucket> and count up total keys
       so we can check that every vbucket has the correct
       number of keys */
    int total = 0;
    for (int i = 0; i < num_vbuckets; i++) {
        for (int j = 0 ; j < num_keys_per_vbucket ; j++) {
            if (keys[i][j] != 0) {
                printf("%s %d\n", keys[i][j], i);
                total++;
            }
        }
    }

    if (total < (num_vbuckets * num_keys_per_vbucket)) {
        fprintf(stderr, "some vbuckets don't have enough keys\n");
        rval = 1;
    }

    vbucket_config_destroy(vb);

    return rval;
}
