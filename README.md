# jank-nrepl-server

[![Build and Load](https://github.com/kylc/jank-nrepl-server/actions/workflows/ci.yaml/badge.svg)](https://github.com/kylc/jank-nrepl-server/actions/workflows/ci.yaml)

## Build & Run

### Babashka

```sh
bb run server
```

### Manual

```sh
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild
cmake --build build

./build/jank-wrapper run-main nrepl-server.core
```
