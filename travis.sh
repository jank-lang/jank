#/usr/bin/env bash

set -eu

# TODO: Also test release builds.
meson setup build -Db_coverage=true
ninja -C build
./build/test/jank-unit-tests
