libvbucket: a vbucket library for memcached
===========================================

Config syntax
-------------

The configuration string is JSON.

Example:

    {
      "hashAlgorithm": "CRC",
      "numReplicas": 2,
      "serverList": ["server1:11211", "server2:11210", "server3:11211"],
      "vBucketMap":
        [
          [0, 1, 2],
          [1, 2, 0],
          [2, 1, -1],
          [1, 2, 0]
        ]
    }

### hashAlgorithm

The hash algorithm can be in upper or lower case. Libvbucket currently
supports the following hash algorithms (via libhashkit):

* default - this is libhashkit's default of one_at_a_time
* md5
* crc - this is CRC32, a good general purpose hash for short strings
* fnv1_64
* fnv1a_64
* fnv1_32
* fnv1a_32
* hsieh
* murmur
* jenkins

### numReplicas

numReplicas is the number of extra copies that will be stored on
servers. Each vBucket in vBucketMap must have this number of server
indexes plus one (the master server).

### serverList

This has one string per server, in whatever format your app expects a
server config string in. This may change to be more structured later.

### vBucketMap

This contains one entry per vBucket, and the number of entries must be
a power of two. Each entry must be an array of numReplicas+1
zero-based indices into serverList, with the first entry indicating
the master server for the bucket and the remaining entries specifying
the replicas, in order. -1 indicates that no server is mapped for that
particular master/replica of that particular vBucket.

In the future this will probably be extended to support "intermediate
states" of vBuckets which are being migrated.

