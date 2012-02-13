#undef NDEBUG
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libvbucket/vbucket.h>

#include "macros.h"

const char *test_cases[] = {
   "regression-bug2112",
   NULL
};

int main(void) {
   char *root = getenv("srcdir");
   if (root != NULL) {
      char buffer[FILENAME_MAX];
      VBUCKET_CONFIG_HANDLE h;
      char key1[9] = "00000000";
      int v, m, num_replicas, ff;
      const char *master;

      for (ff = 0; test_cases[ff] != NULL; ++ff) {
         sprintf(buffer, "%s/tests/config/%s", root, test_cases[ff]);
         fprintf(stderr, "Running regression test for: %s\n", test_cases[ff]);
         h = vbucket_config_create();
         assert(vbucket_config_parse(h, LIBVBUCKET_SOURCE_FILE, buffer) == 0);

         fprintf(stderr, "Testing a key can be hashed.");
         v = vbucket_get_vbucket_by_key(h, key1, strlen(key1));
         m = vbucket_get_master(h, v);
         master = vbucket_config_get_server(h, m);
         fprintf(stderr, "  key: %s vBucketId: %d master: %s", key1, v, master);
         num_replicas = vbucket_config_get_num_replicas(h);
         if (num_replicas > 0) {
            int j;
            fprintf(stderr, " replicas:");
            for (j = 0; j < num_replicas; j++) {
               int r = vbucket_get_replica(h, v, j);
               if (r >= 0) {
                  const char *replica = vbucket_config_get_server(h, r);
                  fprintf(stderr, " %s", replica == NULL ? "(null)" : replica);
               }
            }
         }
         fprintf(stderr, "\n");
         vbucket_config_destroy(h);
      }
   }

   exit(EXIT_SUCCESS);
}
