libstorjshare
=============

Asynchronous multi-platform C library and daemon for hosting files on the Storj network.

## Build

```bash
./autogen.sh
./configure
make
```

To run tests:
```bash
./test/tests
```

To run command line utility:
```bash
./src/storjshare --help
```

And to install locally:
```
sudo make install
```

### Debian / Ubuntu (16.04) Dependencies:

Development tools:
```bash
apt-get install build-essential libtool autotools-dev automake
```

Dependencies:
```bash
apt-get install libcurl4-gnutls-dev libjson-c-dev libsqlite3-dev libuv1-dev libmicrohttpd
```

### OS X Dependencies (w/ homebrew):

Development tools:
```bash
brew install libtool automake pkgconfig
```

Dependencies:
```bash
brew install curl json-c sqlite libuv libmicrohttpd
```

Additional Dependencies:
- libsecp256k1 (https://github.com/bitcoin-core/secp256k1)
- bip32 (https://github.com/libbtc/libbtc)
- minuupnp (https://github.com/miniupnp/miniupnp)

## Architecture

### Daemon Server

  Listens on an external port for hosting files on the Storj network.

  **JSON-RPC API:**

  - `POST /` with commands:
    - `PING` - Request to send back a PONG
    - `MIRROR` - Request to download shard from another farmer
    - `RETRIEVE` - Request to download a shard and responds with a token
    - `ALLOC` - Request to upload a shard and responds with a token
    - `RENEW` - Request to renew length of shard storage

  **SHARD HTTP API:**

  - `GET /shards/<shard-hash>?token=<token>` - Download a shard
  - `POST /shards/<shard-hash>?token=<token>` - Upload a shard

### Control Interface

  Listens on local port or socket that can be used to get information about the daemon, and issue control commands. Optional authentication for remote control abilities and information.

  **JSON-RPC API:**
  - `POST /` with commands:
    - `STATS` - Gives information such as: status, location, uptime, restarts, bridges, offers, shared, delta and port

### Shard Storage

Each shard is saved in a directory structure, the shard hash is split into chunks and the shard is stored in directories with this structure:

```
↳ <2-bytes> directory - first two bytes of shard hash used as the directory name
  ↳ <2-bytes> directory - next two bytes of the shard has used as the directory name
    ↳ <16-bytes> file - the last remaining bytes used for the filename
```

- Maximum total directories is 16 ^ 4 = 65,536
- With 100,000 files in each it would be 6,553,600,000 files total

The goal is to minimize number of folders and number of files per folder, but typically have multiple files in a folder. A collision of the first 4 bytes of the hash is unlikely but not too improbable.

### SQLite Database

Shard contracts table:

```
--------------------------------------------------------------------------------------------------------
| name | data_hash | data_size | store_begin | store_end | renter_hd_key | renter_hd_index | renter_id |
--------------------------------------------------------------------------------------------------------
| type | bytes(20) | int64     | int64       | int64     | bytes         | int             | bytes(20) |
--------------------------------------------------------------------------------------------------------
```

Disk usage table:
```
---------------------------------
| name | total_size  | date     |
---------------------------------
| type | int64       | int64    |
---------------------------------
```

Shard tokens table:
```
------------------------------------------------------------------------------------
| name | token | data_hash | renter_hd_key | renter_hd_index | renter_id | expires |
------------------------------------------------------------------------------------
| type | bytes | bytes     | bytes         | int             | bytes     | int64   |
------------------------------------------------------------------------------------
```
