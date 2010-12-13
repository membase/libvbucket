#undef NDEBUG
#include <assert.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libvbucket/vbucket.h>

int main(void) {
   char *root = getenv("srcdir");
   if (root != NULL) {
      char buffer[PATH_MAX];
      sprintf(buffer, "%s/tests/regression", root);
      DIR *dp = opendir(buffer);
      if (dp == NULL) {
         fprintf(stderr, "Skipping regression check\nFailed to open %s: %s\n",
                 buffer, strerror(errno));
         return 0;
      }
      struct dirent *de;

      while ((de = readdir(dp)) != NULL) {
         if (strncmp(de->d_name, "bug", 3) == 0 || strncmp(de->d_name, "mb-", 3) == 0) {
            sprintf(buffer, "%s/tests/regression/%s", root, de->d_name);
            fprintf(stderr, "Running regression test for: %s\n", de->d_name);
            VBUCKET_CONFIG_HANDLE h = vbucket_config_parse_file(buffer);
            assert(h != NULL);

            if (strcmp(de->d_name, "mb-3161") == 0) {
               assert(strcmp(vbucket_config_get_user(h), "foo") == 0);
               assert(strcmp(vbucket_config_get_password(h), "bar") == 0);
            } else {
               fprintf(stderr, "Testing a key can be hashed.");
               char key1[9] = "00000000";
               int v = vbucket_get_vbucket_by_key(h, key1, strlen(key1));
               int m = vbucket_get_master(h, v);
               const char *master = vbucket_config_get_server(h, m);
               fprintf(stderr, "  key: %s vBucketId: %d master: %s", key1, v, master);
               int num_replicas = vbucket_config_get_num_replicas(h);
               if (num_replicas > 0) {
                  fprintf(stderr, " replicas:");
                  for (int j = 0; j < num_replicas; j++) {
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
