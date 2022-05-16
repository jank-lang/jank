#/usr/bin/env bash

set -eu

which clang++

# TODO: Also test release builds.
rm -rf build
meson setup build -Db_coverage=true
ninja compile -C build

lcov --directory build --capture --initial --output-file "$PWD/build/meson-logs/capture.initial" --gcov-tool "$PWD/bin/ci/llvm-cov"

ninja test -C build

# Code coverage report.
lcov --directory build --capture --no-checksum --rc lcov_branch_coverage=1 --output-file "$PWD/build/meson-logs/capture.run" --gcov-tool "$PWD/bin/ci/llvm-cov"
lcov -a build/meson-logs/capture.initial -a "$PWD/build/meson-logs/capture.run" --rc lcov_branch_coverage=1 -o "$PWD/build/meson-logs/capture.raw"
lcov --extract build/meson-logs/capture.raw "$PWD/src/*" "$PWD/include/*" --output-file "$PWD/build/meson-logs/capture.final" --rc lcov_branch_coverage=1
bash <(curl -s https://codecov.io/bash) -f "$PWD/build/meson-logs/capture.final"
