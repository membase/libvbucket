libvbucket: a vbucket library for memcached
===========================================

Config Syntax
-------------

libvbucket uses JSON for it's configuration format. The directory
`tests/config` contains sample configurations for [vbucket][1] and
[ketama][2] distribution algorithms taken from a live Couchbase
cluster.

Envelope Section
----------------

Configurations can have an optional envelope. If it is absent,
libvbucket will treat the config stream as a VBucket section.

### name

The username which should be used for nodes.  This will be used for both
authentication in HTTP and over memcached binary protocol.  Note that 
"default" is a special bucket which does not use HTTP auth or memcached
protocol auth.  Specifying "default" and empty string for password will
configure the client to use the default bucket.

    "name": "default",

### saslPassword

The password which should be used for nodes, which need authentication.

    "saslPassword": "",

### nodeLocator

The distribution algorithm which will be used to map keys to nodes. If this
field is absent the `vbucket` locator will be used. The possible values
are `ketama` and `vbucket`.

    "nodeLocator": "vbucket",

### vBucketServerMap

The section which describes the vbucket configuration. See the "VBucket
Section" below.

### nodes

The list of objects which describe the nodes in the cluster. The
parser will look for `couchApiBase`, `hostname` and `ports` fields in
this entry.

* hostname

  The hostname and port of the cluster's REST interface.

      "hostname": "172.16.16.76:8091",

* couchApiBase

  The endpoint for CouchDB REST interface. This endpoint should be used
  for Couch view execution.

      "couchApiBase": "http://172.16.16.76:8091/default",

* ports

  List of node ports. Currenty it contains `direct` and `proxy` port to
  connect to the node using memcached protocol. The proxy port could be
  used for vbucket unaware, legacy clients.

      "ports": {
        "proxy": 11211,
        "direct": 11210
      },

VBucket Section
---------------

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

The hash algorithm can be in upper or lower case. libvbucket currently
uses a CRC32 hashing algorithm, a good general purpose hash for short
strings.  The hash algorithm will typically be ketama for memcached
type buckets.

### numReplicas

numReplicas is the number of extra copies that will be stored on
servers. Each vBucket in vBucketMap must have this number of server
indexes plus one (the master server).

### serverList

This has one string per server, in whatever format your app expects a
server config string in. This may change to be more structured later.

### vBucketMap

This contains one entry per vBucket, and the number of entries must be
a power of two. Each entry must be an array of `numReplicas+1`
zero-based indices into serverList, with the first entry indicating
the master server for the bucket and the remaining entries specifying
the replicas, in order. `-1` indicates that no server is mapped for that
particular master/replica of that particular vBucket.

Note there is also a vBucketMapForward which can be sent by the server
in the case that changes are occurring.  The vBucketMapForward indicates
what the future state of the cluster layout will be.

[1]: https://github.com/membase/libvbucket/blob/master/tests/config/vbucket-eight-nodes
[2]: https://github.com/membase/libvbucket/blob/master/tests/config/ketama-eight-nodes
