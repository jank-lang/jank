# ---- libfolly.a ----
# Folly is well and truly a pain in the ass to build through its
# own build system. It regularly fails to build for me
# on all sorts of standard systems (both Linux and macOS) and jank
# has been running with custom patches for several months now. After
# running into compilation issues with it yet again, I've decided to
# just rip the good bits out and only compile what we need.
#
# The Folly fork we have makes things even easier to build by commenting
# out some bits related to jemalloc and tcmalloc. Folly has a whole
# complex preprocessor system for this stuff, but it still ends up
# linking to them even when you say you don't want them.

set(
  folly_sources
  third-party/folly/folly/ScopeGuard.cpp
  third-party/folly/folly/SharedMutex.cpp
  third-party/folly/folly/concurrency/CacheLocality.cpp
  third-party/folly/folly/synchronization/ParkingLot.cpp
  third-party/folly/folly/synchronization/SanitizeThread.cpp
  third-party/folly/folly/lang/SafeAssert.cpp
  third-party/folly/folly/lang/ToAscii.cpp
  third-party/folly/folly/system/ThreadId.cpp
  third-party/folly/folly/system/AtFork.cpp
  third-party/folly/folly/detail/Futex.cpp
  third-party/folly/folly/memory/folly_stubs.cpp
)

if(WIN32)
  list(APPEND folly_sources
    third-party/folly/folly/net/NetOps.cpp
    third-party/folly/folly/net/detail/SocketFileDescriptorMap.cpp
    third-party/folly/folly/portability/Sockets.cpp
    third-party/folly/folly/portability/SysResource.cpp
    third-party/folly/folly/portability/Unistd.cpp
  )
endif()

add_library(folly_lib STATIC ${folly_sources})
set_source_files_properties(${folly_sources} PROPERTIES SKIP_LINTING ON)
target_include_directories(
  folly_lib
  SYSTEM
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/folly>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost-preprocessor/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/boost-process/include>"
)
set_property(TARGET folly_lib PROPERTY OUTPUT_NAME folly)
target_compile_features(folly_lib PUBLIC ${jank_cxx_standard})
target_compile_options(folly_lib PUBLIC ${jank_common_compiler_flags} ${jank_aot_compiler_flags} -w)
target_link_options(folly_lib PRIVATE ${jank_linker_flags})
set_target_properties(folly_lib PROPERTIES ENABLE_EXPORTS 1)
# ---- libfolly.a ----
