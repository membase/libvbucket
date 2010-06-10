#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libvbucket/vbucket.h>

static const char *config =
"{\n"
"  \"hashAlgorithm\": \"CRC\",\n"
"  \"numReplicas\": 2,\n"
"  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server3:11211\"],\n"
"  \"vBucketMap\":\n"
"    [\n"
"      [0, 1, 2],\n"
"      [1, 2, 0],\n"
"      [2, 1, -1],\n"
"      [1, 2, 0]\n"
"    ]\n"
"}";

static const char *configFlat =
"{"
"  \"hashAlgorithm\": \"CRC\","
"  \"numReplicas\": 2,"
"  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server3:11211\"],"
"  \"vBucketMap\":"
"    ["
"      [0, 1, 2],"
"      [1, 2, 0],"
"      [2, 1, -1],"
"      [1, 2, 0]"
"    ]"
"}";

static const char *configInEnvelope =
"{ \"otherKeyThatIsIgnored\": 12345,\n"
  "\"vbucketServerMap\": \n"
    "{\n"
    "  \"hashAlgorithm\": \"CRC\",\n"
    "  \"numReplicas\": 2,\n"
    "  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server3:11211\"],\n"
    "  \"vBucketMap\":\n"
    "    [\n"
    "      [0, 1, 2],\n"
    "      [1, 2, 0],\n"
    "      [2, 1, -1],\n"
    "      [1, 2, 0]\n"
    "    ]\n"
    "}"
"}";

static const char *configInEnvelope2 =
"{\"name\":\"default\",\"uri\":\"/pools/default/buckets/default\","
 "\"streamingUri\":\"/pools/default/bucketsStreaming/default\","
 "\"flushCacheUri\":\"/pools/default/buckets/default/controller/doFlush\","
 "\"nodes\":[{\"clusterMembership\":\"inactiveAdded\",\"status\":\"unhealthy\","
             "\"hostname\":\"127.0.0.1\",\"version\":\"unknown\",\"os\":\"unknown\","
             "\"ports\":{\"proxy\":11213,\"direct\":11212},"
             "\"uptime\":\"0\",\"memoryTotal\":0,\"memoryFree\":0,\"mcdMemoryReserved\":64,\"mcdMemoryAllocated\":0}],"
 "\"stats\":{\"uri\":\"/pools/default/buckets/default/stats\"},"
 "\"vbucketServerMap\": "
    "{"
    "  \"hashAlgorithm\": \"CRC\","
    "  \"numReplicas\": 2,"
    "  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server3:11211\"],"
    "  \"vBucketMap\":"
    "    ["
    "      [0, 1, 2],"
    "      [1, 2, 0],"
    "      [2, 1, -1],"
    "      [1, 2, 0]"
    "    ]"
 "},"
 "\"basicStats\":{\"cacheSize\":64,\"opsPerSec\":0.0,\"evictionsPerSec\":0.0,\"cachePercentUsed\":0.0}}";

struct key_st {
    char *key;
    int vbucket;
};

static const struct key_st keys[] =
{
    { "hello", 0 },
    { "doctor", 0 },
    { "name", 3 },
    { "continue", 3 },
    { "yesterday", 0 },
    { "tomorrow", 1 },
    { "another key", 2 },
    { NULL, -1 }
};

static const char *servers[] = { "server1:11211", "server2:11210", "server3:11211" };

struct vb_st {
    int master;
    int replicas[2];
};

static const struct vb_st vbuckets[] =
{
    { 0, { 1, 2 } },
    { 1, { 2, 0 } },
    { 2, { 1, -1 } },
    { 1, { 2, 0 } }
};

static void testConfig(const char *c) {
    VBUCKET_CONFIG_HANDLE vb = vbucket_config_parse_string(c);
    if (vb == NULL) {
        fprintf(stderr, "vbucket_config_parse_string error: %s\n", vbucket_get_error());
        abort();
    }

    int whoops = 0;
    const struct key_st *k;
    int i = 0;
    while ((k = &keys[i++])->key != NULL) {
        int id = vbucket_get_vbucket_by_key(vb, k->key, strlen(k->key));
        if (id != k->vbucket) {
            fprintf(stderr, "Expected vbucket %d for key '%s' but got %d\n",
                    k->vbucket, k->key, id);
            whoops = 1;
        }
    }

    if (whoops) {
        abort();
    }

    assert(vbucket_config_get_num_servers(vb) == 3);
    assert(vbucket_config_get_num_replicas(vb) == 2);

    for (i = 0; i < 3; ++i) {
        assert(strcmp(vbucket_config_get_server(vb, i), servers[i]) == 0);
    }

    for (i = 0; i < 4; ++i) {
        assert(vbucket_get_master(vb, i) == vbuckets[i].master);
        assert(vbucket_get_replica(vb, i, 0) == vbuckets[i].replicas[0]);
        assert(vbucket_get_replica(vb, i, 1) == vbuckets[i].replicas[1]);
    }

    vbucket_config_destroy(vb);
}

int main(void) {
  testConfig(config);
  testConfig(configFlat);
  testConfig(configInEnvelope);
  testConfig(configInEnvelope2);
}


