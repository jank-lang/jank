#/usr/bin/env bash

set -eu

# TODO: Also test release builds.
rm -rf build
meson setup build
meson compile -C build

# Code coverage.
LLVM_PROFILE_FILE=build/test.profraw ./build/test/jank-unit-tests
llvm-profdata merge --sparse build/test.profraw -o build/test.profdata
llvm-cov show ./build/test/jank-unit-tests --instr-profile build/test.profdata > coverage.txt
bash <(curl -s https://codecov.io/bash) -f coverage.txt
