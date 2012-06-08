/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

#undef NDEBUG
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <libvbucket/vbucket.h>
#include "src/hash.h"
#include "macros.h"

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

const char *test_cases[] = {
    "ketama-eight-nodes",
    "ketama-ordered-eight-nodes",
    NULL
};

int main(void) {
    char *root = getenv("srcdir");
    const char *host;
    char buffer[FILENAME_MAX];
    char key[NKEY];
    int idx, i, len, ff;
    VBUCKET_CONFIG_HANDLE vb;
    unsigned char checksum[16];
    unsigned char expected[16];
    void *ctx;

    if (root != NULL) {
        for (ff = 0; test_cases[ff] != NULL; ++ff) {
            snprintf(buffer, FILENAME_MAX, "%s/tests/config/%s", root, test_cases[ff]);
            fprintf(stderr, "Running ketama test for: %s\n", test_cases[ff]);
            vb = vbucket_config_create();
            assert(vbucket_config_parse(vb, LIBVBUCKET_SOURCE_FILE, buffer) == 0);
            /* check if it conforms to libketama results */
            snprintf(buffer, FILENAME_MAX,"%s/tests/config/%s.md5sum", root, test_cases[ff]);
            read_checksum(buffer, expected);
            memset(checksum, 0, 16);
            ctx = NULL;

            for (i = 0; i < 1000000; i++) {
                len = snprintf(key, NKEY, "%d", i);
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

    exit(EXIT_SUCCESS);
}
