/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

#undef NDEBUG
#include <assert.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libvbucket/vbucket.h>
#include "src/hash.h"

#define NKEY 10

static void read_checksum(const char *path, unsigned char *result) {
    FILE *ff = fopen(path, "r");
    int ii, nn;
    for (ii = 0; ii < 16; ++ii) {
        nn = fscanf(ff, "%2x", (unsigned int *)&result[ii]);
        if (nn != 1) {
            perror("scanf");
            abort();
        }
    }
}

int main(void) {
    char *root = getenv("srcdir");
    DIR *dp;
    const char *host;
    char buffer[PATH_MAX];
    char key[NKEY];
    int idx, i, len;
    struct dirent *de;
    VBUCKET_CONFIG_HANDLE vb;
    unsigned char checksum[16];
    unsigned char expected[16];
    void *ctx;

    if (root != NULL) {
        sprintf(buffer, "%s/tests/config", root);
        dp = opendir(buffer);
        if (dp == NULL) {
            fprintf(stderr, "Skipping ketama check\nFailed to open %s: %s\n",
                    buffer, strerror(errno));
            return 0;
        }

        while ((de = readdir(dp)) != NULL) {
            if (strncmp(de->d_name, "ketama", 6) == 0 && strchr(de->d_name, '.') == NULL) {
                sprintf(buffer, "%s/tests/config/%s", root, de->d_name);
                fprintf(stderr, "Running ketama test for: %s\n", de->d_name);
                vb = vbucket_config_parse_file(buffer);
                assert(vb != NULL);

                /* check if it conforms to libketama results */
                sprintf(buffer, "%s/tests/config/%s.md5sum", root, de->d_name);
                read_checksum(buffer, expected);
                memset(checksum, 0, 16);
                ctx = NULL;

                for (i = 0; i < 1000000; i++) {
                    len = sprintf(key, "%d", i);
                    vbucket_map(vb, key, len, NULL, &idx);
                    host = vbucket_config_get_server(vb, idx);
                    ctx = hash_md5_update(ctx, host, strlen(host));
                }
                hash_md5_final(ctx, checksum);

                for (i = 0; i < 16; i++) {
                    assert(checksum[i] == expected[i]);
                }

                vbucket_config_destroy(vb);
            }
        }
        closedir(dp);
    }

    return 0;
}
