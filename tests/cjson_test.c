/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include <stdio.h>

int main(void) {
   const char *expected = "{\"foo\":\"bar\"}";
   char *str;
   int retcode = EXIT_SUCCESS;
   cJSON *obj;

   obj = cJSON_CreateObject();
   cJSON_AddStringToObject(obj, "foo", "bar");
   str = cJSON_PrintUnformatted(obj);
   if (strcmp(str, expected) != 0) {
      fprintf(stderr, "Expected %s got %s\n", expected, str);
      retcode = EXIT_FAILURE;
   }
   cJSON_Delete(obj);
   cJSON_Free(str);

   return retcode;
}
