#/bin/bash

set -e

if [[ "$analyze" == "true" ]]; then
  scan_build=scan-build
else
  scan_build=
fi

# Release build
mkdir build_release ; cd build_release
$scan_build cmake .. -DCMAKE_BUILD_TYPE=RELEASE
$scan_build make -j2
$scan_build make jank_test
cd ..

# Debug build
mkdir build_debug ; cd build_debug
$scan_build cmake .. -DCMAKE_BUILD_TYPE=DEBUG
$scan_build make -j2
$scan_build make jank_test
