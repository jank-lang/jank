set(
  CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/third-party/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file"
)

option(jank_tests "Build jank's tests" OFF)
if(jank_tests)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

# Folly can fail, on Linux, with clang, since it struggles to find things like clock_gettime.
# We get around that by just trusting they're present.
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -DFOLLY_HAVE_RECVMMSG -DFOLLY_HAVE_CLOCK_GETTIME")

# More portable binaries would be preferable, so static linkage where possible.
set(VCPKG_LIBRARY_LINKAGE static)

# Normally, vcpkg builds both debug and release builds. We only need release the vast
# majority of the time.
set(VCPKG_BUILD_TYPE release)
