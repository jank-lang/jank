# jank-nrepl-server

```sh
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -B build
cmake --build build

jank -Isrc/cpp/ -Lbuild/ -lnrepl_server run-main --module-path src/jank/ nrepl-server.core
```
