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
         if (strncmp(de->d_name, "bug", 3) == 0) {
            sprintf(buffer, "%s/tests/regression/%s", root, de->d_name);
            fprintf(stderr, "Running regression test for: %s\n", de->d_name);
            VBUCKET_CONFIG_HANDLE h = vbucket_config_parse_file(buffer);
            assert(h != NULL);
            vbucket_config_destroy(h);
         }
      }
      closedir(dp);
   }

   return 0;
}
