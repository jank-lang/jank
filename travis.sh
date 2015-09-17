#/bin/bash

set -e

if [[ "$analyze" == "true" ]]; then
  scan_build="scan-build --use-cc=$(which $CC) --use-c++ --use-c++=$(which $CXX)"
  analyze_suffix=_analyze

  if [[ "$CXX" == "g++" ]]; then
    # No need to analyze for both gcc and clang
    exit 0
  fi
else
  scan_build=
  analyze_suffix=
fi

# Release build
dir=build_release$analyze_suffix
mkdir $dir ; cd $dir
$scan_build cmake .. -DCMAKE_BUILD_TYPE=RELEASE
$scan_build make -j2
$scan_build make jank_test
cd ..

# Debug build
dir=build_debug$analyze_suffix
mkdir $dir ; cd $dir
$scan_build cmake .. -DCMAKE_BUILD_TYPE=DEBUG
$scan_build make -j2
$scan_build make jank_test
