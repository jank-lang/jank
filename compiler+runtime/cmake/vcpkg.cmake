set(
  CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/third-party/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file"
)

option(jank_tests "Build jank's tests" OFF)
if(jank_tests)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()
