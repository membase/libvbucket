#undef NDEBUG
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libvbucket/vbucket.h>

#include "macros.h"

int main(void) {
   char *root = getenv("srcdir");
   if (root != NULL) {
      char buffer[PATH_MAX];
      DIR *dp;
      struct dirent *de;

      sprintf(buffer, "%s/tests/config", root);
      dp = opendir(buffer);
      if (dp == NULL) {
         fprintf(stderr, "Skipping regression check\nFailed to open %s: %s\n",
                 buffer, strerror(errno));
         return 0;
      }

      while ((de = readdir(dp)) != NULL) {
         if (strncmp(de->d_name, "regression-bug", 14) == 0 || strncmp(de->d_name, "mb-", 3) == 0) {
            VBUCKET_CONFIG_HANDLE h;
            sprintf(buffer, "%s/tests/config/%s", root, de->d_name);
            fprintf(stderr, "Running regression test for: %s\n", de->d_name);
            h = vbucket_config_parse_file(buffer);
            assert(h != NULL);

            if (strcmp(de->d_name, "mb-3161") == 0) {
               assert(strcmp(vbucket_config_get_user(h), "foo") == 0);
               assert(strcmp(vbucket_config_get_password(h), "bar") == 0);
            } else {
               char key1[9] = "00000000";
               int v, m, num_replicas;
               const char *master;

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
            }
            vbucket_config_destroy(h);
         }
      }
      closedir(dp);
   }

   return 0;
}
