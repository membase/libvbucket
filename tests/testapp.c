/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

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
  "\"vBucketServerMap\": \n"
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

static const char *configInEnvelopeFFT =
"{ \"otherKeyThatIsIgnored\": 12345,\n"
  "\"vBucketServerMap\": \n"
    "{\n"
    "  \"hashAlgorithm\": \"CRC\",\n"
    "  \"numReplicas\": 2,\n"
    "  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server3:11211\", \"server4:11211\"],\n"
    "  \"vBucketMap\":\n"
    "    [\n"
    "      [0, 1, 2],\n"
    "      [1, 2, 0],\n"
    "      [2, 1, -1],\n"
    "      [1, 2, 0]\n"
    "    ],\n"
    "  \"vBucketMapForward\":\n"
    "    [\n"
    "      [3, 0, 0],\n"
    "      [2, 1, 3],\n"
    "      [1, 2, 2],\n"
    "      [0, 3, 1]\n"
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
 "\"vBucketServerMap\": "
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

    assert(vbucket_config_get_num_servers(vb) == 3 || vbucket_config_get_num_servers(vb) == 4);
    assert(vbucket_config_get_num_replicas(vb) == 2);

    for (i = 0; i < 3; ++i) {
        assert(strcmp(vbucket_config_get_server(vb, i), servers[i]) == 0);
    }

    for (i = 0; i < 4; ++i) {
        assert(vbucket_get_master(vb, i) == vbuckets[i].master);
        assert(vbucket_get_replica(vb, i, 0) == vbuckets[i].replicas[0]);
        assert(vbucket_get_replica(vb, i, 1) == vbuckets[i].replicas[1]);
    }

    assert(vbucket_config_get_user(vb) == NULL);
    assert(vbucket_config_get_password(vb) == NULL);

    vbucket_config_destroy(vb);
}


static void testWrongServer(const char *c) {
    VBUCKET_CONFIG_HANDLE vb = vbucket_config_parse_string(c);
    if (vb == NULL) {
        fprintf(stderr, "vbucket_config_parse_string error: %s\n", vbucket_get_error());
        abort();
    }

    // Starts at 0
    assert(vbucket_get_master(vb, 0) == 0);
    // Does not change when I told it I found the wrong thing
    assert(vbucket_found_incorrect_master(vb, 0, 1) == 0);
    assert(vbucket_get_master(vb, 0) == 0);
    // Does change if I tell it I got the right thing and it was wrong.
    assert(vbucket_found_incorrect_master(vb, 0, 0) == 1);
    assert(vbucket_get_master(vb, 0) == 1);
    // ...and again
    assert(vbucket_found_incorrect_master(vb, 0, 1) == 2);
    assert(vbucket_get_master(vb, 0) == 2);
    // ...and then wraps
    assert(vbucket_found_incorrect_master(vb, 0, 2) == 0);
    assert(vbucket_get_master(vb, 0) == 0);

    vbucket_config_destroy(vb);
}

static void testWrongServerFFT(const char *c) {

    VBUCKET_CONFIG_HANDLE vb = vbucket_config_parse_string(c);
    int rv = 0;
    int nvb = 0;
    int i = 0;

    if (vb == NULL) {
        fprintf(stderr, "vbucket_config_parse_string error: %s\n", vbucket_get_error());
        abort();
    }

    // found incorrect master should not be the same as get master now
    nvb = vbucket_config_get_num_vbuckets(vb);
    for (i = 0; i < nvb; i++) {
        rv = vbucket_get_master(vb, i);
        assert(rv != vbucket_found_incorrect_master(vb, i, rv));
    }
    // the ideal test case should be that we check that the vbucket
    // and the fvbucket map are identical at this point. TODO untill
    // we have a vbucketlib function that diffs vbuckets and fvbuckets
    vbucket_config_destroy(vb);
}

static void testConfigDiff(void) {
    const char *cfg1 = "{\n"
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
    const char *cfg2 = "{\n"
        "  \"hashAlgorithm\": \"CRC\",\n"
        "  \"numReplicas\": 2,\n"
        "  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server4:11211\"],\n"
        "  \"vBucketMap\":\n"
        "    [\n"
        "      [0, 1, 2],\n"
        "      [1, 2, 0],\n"
        "      [2, 1, -1],\n"
        "      [0, 2, 0]\n"
        "    ]\n"
        "}";
    const char *cfg3 = "{\n"
        "  \"hashAlgorithm\": \"CRC\",\n"
        "  \"numReplicas\": 1,\n"
        "  \"serverList\": [\"server1:11211\", \"server2:11210\"],\n"
        "  \"vBucketMap\":\n"
        "    [\n"
        "      [0, 1],\n"
        "      [1, 0],\n"
        "      [1, 0],\n"
        "      [0, 1],\n"
        "      [0, 1],\n"
        "      [1, 0],\n"
        "      [1, 0],\n"
        "      [0, 1]\n"
        "    ]\n"
        "}";

    VBUCKET_CONFIG_HANDLE vb1 = vbucket_config_parse_string(cfg1);
    assert(vb1);

    VBUCKET_CONFIG_HANDLE vb2 = vbucket_config_parse_string(cfg2);
    assert(vb2);

    VBUCKET_CONFIG_DIFF *diff = vbucket_compare(vb1, vb2);
    assert(diff);

    assert(diff->sequence_changed);
    assert(diff->n_vb_changes == 1);
    assert(strcmp(diff->servers_added[0], "server4:11211") == 0);
    assert(diff->servers_added[1] == NULL);
    assert(strcmp(diff->servers_removed[0], "server3:11211") == 0);
    assert(diff->servers_removed[1] == NULL);

    vbucket_free_diff(diff);
    vbucket_config_destroy(vb2);

    vb2 = vbucket_config_parse_string(cfg3);
    assert(vb2);

    diff = vbucket_compare(vb1, vb2);
    assert(diff);

    assert(diff->sequence_changed);
    assert(diff->n_vb_changes == -1);
    assert(diff->servers_added[0] == NULL);
    assert(strcmp(diff->servers_removed[0], "server3:11211") == 0);
    assert(diff->servers_removed[1] == NULL);
}

static void testConfigDiffSame(void) {
    VBUCKET_CONFIG_HANDLE vb1 = vbucket_config_parse_string(config);
    assert(vb1);

    VBUCKET_CONFIG_HANDLE vb2 = vbucket_config_parse_string(config);
    assert(vb2);

    VBUCKET_CONFIG_DIFF *diff = vbucket_compare(vb1, vb2);
    assert(diff);

    assert(diff->sequence_changed == 0);
    assert(diff->n_vb_changes == 0);
    assert(diff->servers_added[0] == NULL);
    assert(diff->servers_removed[0] == NULL);

    vbucket_free_diff(diff);
    vbucket_config_destroy(vb1);
    vbucket_config_destroy(vb2);
}

static void testConfigUserPassword(void) {
    const char *cfg1 = "{\n"
        "  \"hashAlgorithm\": \"CRC\",\n"
        "  \"numReplicas\": 2,\n"
        "  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server3:11211\"],\n"
        "  \"name\": \"theUser\",\n"
        "  \"saslPassword\": \"thePassword\",\n"
        "  \"vBucketMap\":\n"
        "    [\n"
        "      [0, 1, 2],\n"
        "      [1, 2, 0],\n"
        "      [2, 1, -1],\n"
        "      [1, 2, 0]\n"
        "    ]\n"
        "}";

    const char *cfg2 = "{\n"
        "  \"hashAlgorithm\": \"CRC\",\n"
        "  \"numReplicas\": 2,\n"
        "  \"serverList\": [\"server1:11211\", \"server2:11210\", \"server3:11211\"],\n"
        "  \"name\": \"theUserIsDifferent\",\n"
        "  \"saslPassword\": \"thePasswordIsDifferent\",\n"
        "  \"vBucketMap\":\n"
        "    [\n"
        "      [0, 1, 2],\n"
        "      [1, 2, 0],\n"
        "      [2, 1, -1],\n"
        "      [1, 2, 0]\n"
        "    ]\n"
        "}";

    VBUCKET_CONFIG_HANDLE vb1 = vbucket_config_parse_string(cfg1);
    assert(vb1);
    assert(strcmp(vbucket_config_get_user(vb1), "theUser") == 0);
    assert(strcmp(vbucket_config_get_password(vb1), "thePassword") == 0);

    VBUCKET_CONFIG_HANDLE vb2 = vbucket_config_parse_string(cfg2);
    assert(vb2);
    assert(strcmp(vbucket_config_get_user(vb2), "theUserIsDifferent") == 0);
    assert(strcmp(vbucket_config_get_password(vb2), "thePasswordIsDifferent") == 0);

    VBUCKET_CONFIG_DIFF *diff = vbucket_compare(vb1, vb2);
    assert(diff);

    assert(diff->sequence_changed);
    assert(diff->n_vb_changes == 0);
    assert(diff->servers_added[0] == NULL);
    assert(diff->servers_removed[0] == NULL);

    vbucket_free_diff(diff);

    diff = vbucket_compare(vb1, vb1);
    assert(diff);

    assert(diff->sequence_changed == 0);
    assert(diff->n_vb_changes == 0);
    assert(diff->servers_added[0] == NULL);
    assert(diff->servers_removed[0] == NULL);

    vbucket_free_diff(diff);

    vbucket_config_destroy(vb1);
    vbucket_config_destroy(vb2);
}

static void testConfigCouchApiBase(void)
{
    const char *cfg =
        "{                                                             "
        "  \"nodes\": [                                                "
        "    {                                                         "
        "      \"hostname\": \"192.168.2.123:9000\",                   "
        "      \"couchApiBase\": \"http://192.168.2.123:9502/default\","
        "      \"ports\": {                                            "
        "        \"proxy\": 12005,                                     "
        "        \"direct\": 12004                                     "
        "      }                                                       "
        "    },                                                        "
        "    {                                                         "
        "      \"hostname\": \"192.168.2.123:9000\",                   "
        "      \"couchApiBase\": \"http://192.168.2.123:9500/default\","
        "      \"ports\": {                                            "
        "        \"proxy\": 12001,                                     "
        "        \"direct\": 12000                                     "
        "      }                                                       "
        "    },                                                        "
        "    {                                                         "
        "      \"hostname\": \"192.168.2.123:9000\",                   "
        "      \"couchApiBase\": \"http://192.168.2.123:9501/default\","
        "      \"ports\": {                                            "
        "        \"proxy\": 12003,                                     "
        "        \"direct\": 12002                                     "
        "      }                                                       "
        "    }                                                         "
        "  ],                                                          "
        "  \"vBucketServerMap\": {                                     "
        "    \"hashAlgorithm\": \"CRC\",                               "
        "    \"numReplicas\": 1,                                       "
        "    \"serverList\": [                                         "
        "      \"192.168.2.123:12000\",                                "
        "      \"192.168.2.123:12002\",                                "
        "      \"192.168.2.123:12004\"                                 "
        "    ],                                                        "
        "    \"vBucketMap\": [                                         "
        "      [ 0, 1 ],                                               "
        "      [ 0, 1 ],                                               "
        "      [ 0, 1 ],                                               "
        "      [ 1, 2 ],                                               "
        "      [ 1, 2 ],                                               "
        "      [ 2, 0 ],                                               "
        "      [ 2, 1 ],                                               "
        "      [ 2, 1 ],                                               "
        "      [ 1, 0 ],                                               "
        "      [ 1, 0 ],                                               "
        "      [ 1, 0 ],                                               "
        "      [ 0, 2 ],                                               "
        "      [ 0, 2 ],                                               "
        "      [ 0, 2 ],                                               "
        "      [ 2, 0 ],                                               "
        "      [ 2, 0 ]                                                "
        "    ]                                                         "
        "  }                                                           "
        "}                                                             ";

    VBUCKET_CONFIG_HANDLE vb = vbucket_config_parse_string(cfg);
    assert(vb);
    assert(strcmp(vbucket_config_get_couch_api_base(vb, 0), "http://192.168.2.123:9500/default") == 0);
    assert(strcmp(vbucket_config_get_couch_api_base(vb, 1), "http://192.168.2.123:9501/default") == 0);
    assert(strcmp(vbucket_config_get_couch_api_base(vb, 2), "http://192.168.2.123:9502/default") == 0);
    assert(strcmp(vbucket_config_get_server(vb, 0), "192.168.2.123:12000") == 0);
    assert(strcmp(vbucket_config_get_server(vb, 1), "192.168.2.123:12002") == 0);
    assert(strcmp(vbucket_config_get_server(vb, 2), "192.168.2.123:12004") == 0);
}

int main(void) {
  testConfig(config);
  testConfig(configFlat);
  testConfig(configInEnvelope);
  testConfig(configInEnvelope2);
  testConfig(configInEnvelopeFFT);
  testWrongServer(config);
  testWrongServerFFT(configInEnvelopeFFT);
  testConfigDiff();
  testConfigDiffSame();
  testConfigUserPassword();
  testConfigCouchApiBase();
}


