# jank-nrepl-server

[![Build and Load](https://github.com/kylc/jank-nrepl-server/actions/workflows/ci.yaml/badge.svg)](https://github.com/kylc/jank-nrepl-server/actions/workflows/ci.yaml)

## Build & Run

### Babashka

```sh
bb run server
```

### Manual

```sh
cmake --workflow --preset release

./build/jank-wrapper run-main nrepl-server.core
```

## Distribute

To compile an AOT executable:

``` sh
./build/jank-wrapper compile nrepl-server.core -o nrepl-server
./nrepl-server
```
