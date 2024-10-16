cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
    FATAL_ERROR
    "In-source builds are not supported. "
    "Please read README.md before trying to build this project. "
    "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
  )

include(cmake/vcpkg.cmake)
# LTO is explicitly not used, since it leads to runtime symbol lookup issues with the JIT.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)

set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/vcpkg-triplet")
set(VCPKG_TARGET_TRIPLET "x64-clang-static")

project(
    jank
    VERSION 0.1
    DESCRIPTION "A native Clojure dialect built on LLVM."
    HOMEPAGE_URL "https://jank-lang.org/"
    # C is needed here, due to a clang issue:
    # https://stackoverflow.com/questions/71740678/cmake-error-in-findterminfo-with-clang-15-on-macos
    LANGUAGES C CXX
)

find_package(Git REQUIRED)
execute_process(
  COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE jank_git_hash
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(jank_version "${CMAKE_PROJECT_VERSION}-${jank_git_hash}")

set(jank_cxx_standard cxx_std_20)

# We need different flags for linking whole libraries on different OSes.
# We need to be mindful to wrap each library with ${jank_link_whole_start} and ${jank_link_whole_end}.
# For example:
#   ${jank_link_whole_start} jank_lib ${jank_link_whole_end}
#   ${jank_link_whole_start} nanobench_lib ${jank_link_whole_end}
#
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(jank_link_whole_start "-Wl,--whole-archive")
  set(jank_link_whole_end "-Wl,--no-whole-archive")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(jank_link_whole_start "-Wl,-force_load")
  set(jank_link_whole_end "")
else()
  message(FATAL_ERROR "Unsupported OS: " ${CMAKE_SYSTEM_NAME})
endif()

set(
  jank_aot_compiler_flags
  -Wall -Wextra -Wpedantic
  -Wfloat-equal -Wuninitialized -Wswitch-enum -Wnon-virtual-dtor
  -Wold-style-cast -Wno-gnu-case-range
  -Wno-gnu-conditional-omitted-operand
  -Wno-implicit-fallthrough
  -Wno-covered-switch-default
  -fno-common
  -frtti
  -fexceptions
  $<$<CONFIG:Debug>:-O0>
  $<$<CONFIG:Release>:-O2>
)

set(
  jank_jit_compiler_flags
  # We don't actually want to see any warnings for JIT compiled code.
  -w
)


# TODO: Remove FMT_HEADER_ONLY once the extern template Clang bug is fixed.
set(
  jank_common_compiler_flags
  -std=gnu++20
  -DIMMER_HAS_LIBGC=1 -DIMMER_TAGGED_NODE=0 -DHAVE_CXX14=1
  -DFMT_HEADER_ONLY=1
  #-DLIBASSERT_USE_MAGIC_ENUM=1
  #-DLIBASSERT_USE_FMT=1
  #-DLIBASSERT_STATIC_DEFINE=1
  #-stdlib=libc++
)

set(
  jank_linker_flags
  $<$<PLATFORM_ID:Darwin>:-L/opt/homebrew/lib>
  #-stdlib=libc++ -lc++abi
  ${CMAKE_SOURCE_DIR}/llvm-install/usr/local/lib/libunwind.a
  ${CMAKE_SOURCE_DIR}/llvm-install/usr/local/lib/libc++abi.a
  )
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND jank_common_compiler_flags #-Werror
    -DJANK_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  list(APPEND jank_common_compiler_flags -DJANK_RELEASE)
endif()

include(cmake/coverage.cmake)
include(cmake/analysis.cmake)

# Enable easier tooling during development.
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# ---- Clang/LLVM ----
include(cmake/llvm.cmake)
llvm_map_components_to_libnames(jank_llvm_libs ${LLVM_TARGETS_TO_BUILD} support lineeditor orcjit native core)

function(jank_hook_llvm target)
  set(tgt "${target}")
  target_include_directories("${tgt}" SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
  target_compile_definitions("${tgt}" PRIVATE ${LLVM_DEFINITIONS})
  target_link_libraries("${tgt}" PRIVATE ${jank_llvm_libs})
endfunction()
# ---- Clang/LLVM ----

# ---- libjank.a ----
# The sources for jank are used for both the compiler and its tests, so
# they're just pulled into a static library. It's important that this
# remains a static lib, since these symbols need to be accessible in the
# compiler's runtime by the JIT compiler.

add_library(
  jank_lib STATIC
  src/cpp/jank/hash.cpp
  src/cpp/jank/util/cli.cpp
  src/cpp/jank/util/sha256.cpp
  src/cpp/jank/util/dir.cpp
  src/cpp/jank/util/mapped_file.cpp
  src/cpp/jank/util/scope_exit.cpp
  src/cpp/jank/util/escape.cpp
  src/cpp/jank/util/clang_format.cpp
  src/cpp/jank/profile/time.cpp
  src/cpp/jank/read/lex.cpp
  src/cpp/jank/read/parse.cpp
  src/cpp/jank/runtime/core.cpp
  src/cpp/jank/runtime/core/to_string.cpp
  src/cpp/jank/runtime/core/seq.cpp
  src/cpp/jank/runtime/core/truthy.cpp
  src/cpp/jank/runtime/core/munge.cpp
  src/cpp/jank/runtime/core/math.cpp
  src/cpp/jank/runtime/perf.cpp
  src/cpp/jank/runtime/module/loader.cpp
  src/cpp/jank/runtime/object.cpp
  src/cpp/jank/runtime/detail/native_persistent_array_map.cpp
  src/cpp/jank/runtime/context.cpp
  src/cpp/jank/runtime/ns.cpp
  src/cpp/jank/runtime/var.cpp
  src/cpp/jank/runtime/obj/nil.cpp
  src/cpp/jank/runtime/obj/number.cpp
  src/cpp/jank/runtime/obj/native_function_wrapper.cpp
  src/cpp/jank/runtime/obj/jit_function.cpp
  src/cpp/jank/runtime/obj/multi_function.cpp
  src/cpp/jank/runtime/obj/symbol.cpp
  src/cpp/jank/runtime/obj/keyword.cpp
  src/cpp/jank/runtime/obj/character.cpp
  src/cpp/jank/runtime/obj/persistent_list.cpp
  src/cpp/jank/runtime/obj/persistent_vector.cpp
  src/cpp/jank/runtime/obj/persistent_vector_sequence.cpp
  src/cpp/jank/runtime/obj/persistent_array_map.cpp
  src/cpp/jank/runtime/obj/persistent_hash_map.cpp
  src/cpp/jank/runtime/obj/transient_hash_map.cpp
  src/cpp/jank/runtime/obj/persistent_sorted_map.cpp
  src/cpp/jank/runtime/obj/transient_sorted_map.cpp
  src/cpp/jank/runtime/obj/transient_vector.cpp
  src/cpp/jank/runtime/obj/persistent_hash_set.cpp
  src/cpp/jank/runtime/obj/transient_hash_set.cpp
  src/cpp/jank/runtime/obj/persistent_sorted_set.cpp
  src/cpp/jank/runtime/obj/transient_sorted_set.cpp
  src/cpp/jank/runtime/obj/persistent_string.cpp
  src/cpp/jank/runtime/obj/persistent_string_sequence.cpp
  src/cpp/jank/runtime/obj/cons.cpp
  src/cpp/jank/runtime/obj/range.cpp
  src/cpp/jank/runtime/obj/iterator.cpp
  src/cpp/jank/runtime/obj/lazy_sequence.cpp
  src/cpp/jank/runtime/obj/chunk_buffer.cpp
  src/cpp/jank/runtime/obj/array_chunk.cpp
  src/cpp/jank/runtime/obj/chunked_cons.cpp
  src/cpp/jank/runtime/obj/native_array_sequence.cpp
  src/cpp/jank/runtime/obj/native_vector_sequence.cpp
  src/cpp/jank/runtime/obj/atom.cpp
  src/cpp/jank/runtime/obj/volatile.cpp
  src/cpp/jank/runtime/obj/reduced.cpp
  src/cpp/jank/runtime/behavior/callable.cpp
  src/cpp/jank/runtime/behavior/metadatable.cpp
  src/cpp/jank/runtime/behavior/derefable.cpp
  src/cpp/jank/analyze/processor.cpp
  src/cpp/jank/analyze/local_frame.cpp
  src/cpp/jank/analyze/step/force_boxed.cpp
  src/cpp/jank/evaluate.cpp
  src/cpp/jank/codegen/processor.cpp
  src/cpp/jank/jit/processor.cpp
)

target_include_directories(
    jank_lib
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/cpp>"
)
target_include_directories(
    jank_lib
    SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/nanobench/include>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/folly>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/bpptree/include>"
)
target_precompile_headers(
  jank_lib
  PUBLIC ${PROJECT_SOURCE_DIR}/include/cpp/jank/prelude.hpp
)

set_property(TARGET jank_lib PROPERTY OUTPUT_NAME jank)

target_compile_features(jank_lib PUBLIC ${jank_cxx_standard})
target_compile_options(jank_lib PUBLIC ${jank_common_compiler_flags} ${jank_aot_compiler_flags})

find_package(Immer CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem preprocessor)
find_package(BDWgc CONFIG REQUIRED)
find_package(libzippp CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
#find_package(libassert CONFIG REQUIRED)

target_include_directories(jank_lib SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(
  jank_lib PRIVATE
  immer fmt::fmt
  BDWgc::gc BDWgc::cord BDWgc::gccpp BDWgc::gctba
  libzippp::libzippp
  CLI11::CLI11
  #libassert::assert
  readline
  clangInterpreter
  Boost::boost
  $<$<PLATFORM_ID:Darwin>:crypto>
  ${CMAKE_SOURCE_DIR}/llvm-install/usr/local/lib/libunwind.a
  ${CMAKE_SOURCE_DIR}/llvm-install/usr/local/lib/libc++abi.a
)

jank_hook_llvm(jank_lib)

# Build a string of all of our JIT compilation flags so we can load them up in the runtime.
set(jank_jit_compile_flags_list ${jank_common_compiler_flags} ${jank_jit_compiler_flags})
list(JOIN jank_jit_compile_flags_list " " jank_jit_compile_flags_str)

# Build a string of all include flags for jank. This will be used for building the PCH at build
# time as well as building the PCH on first run, after installation.
get_target_property(jank_lib_includes jank_lib INCLUDE_DIRECTORIES)
set(jank_lib_includes_str "")
foreach(dir ${jank_lib_includes})
  # System includes are just the path, but normal includes have this format:
  # $<BUILD_INTERFACE:/home/jeaye/projects/jank/compiler+runtime/include/cpp>
  # We need to strip all of that out.
  if(${dir} MATCHES "BUILD_INTERFACE:")
    string(REGEX REPLACE "\\$<BUILD_INTERFACE:(.*)>" "\\1" dir_path ${dir})
    string(APPEND jank_lib_includes_str "-I${dir_path} ")
  else()
    string(APPEND jank_lib_includes_str "-isystem ${dir} ")
  endif()
endforeach()
target_include_directories(
    jank_lib
    SYSTEM
    PUBLIC
    "$<$<PLATFORM_ID:Darwin>:/opt/homebrew/include>"
)


target_compile_options(
  jank_lib
  PUBLIC
  -DJANK_VERSION="${jank_version}"
  -DJANK_JIT_FLAGS="${jank_jit_compile_flags_str}"
  -DJANK_CLANG_PREFIX="${CLANG_INSTALL_PREFIX}"
)
target_link_options(jank_lib PRIVATE ${jank_linker_flags})

#set_target_properties(jank_lib PROPERTIES LINK_FLAGS_RELEASE "-s")

# Symbol exporting for JIT.
set_target_properties(jank_lib PROPERTIES ENABLE_EXPORTS 1)
# ---- libjank.a ----

# ---- libnanobench.a ----
# nanobench uses a single header for both .hpp and .cpp inclusion, based on
# whether a define has been set. This doesn't work with jank's pre-compiled
# headers, so I've manually split the two up and included them here.

set(
  nanobench_sources
  third-party/nanobench/src/nanobench.cpp
)

add_library(nanobench_lib STATIC ${nanobench_sources})

# Not our code. Don't try to lint it.
set_source_files_properties(${nanobench_sources} PROPERTIES SKIP_LINTING ON)

target_include_directories(
    nanobench_lib
    SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/nanobench/include>"
)

set_property(TARGET nanobench_lib PROPERTY OUTPUT_NAME nanobench)

target_compile_features(nanobench_lib PUBLIC ${jank_cxx_standard})
target_compile_options(nanobench_lib PUBLIC ${jank_common_compiler_flags} ${jank_aot_compiler_flags})
target_link_options(nanobench_lib PRIVATE ${jank_linker_flags})

#set_target_properties(nanobench_lib PROPERTIES LINK_FLAGS_RELEASE "-s")

# Symbol exporting for JIT.
set_target_properties(nanobench_lib PROPERTIES ENABLE_EXPORTS 1)
# ---- libnanobench.a ----

# ---- libfolly.a ----
# Folly is well and truly a pain in the ass to build through its
# own build system (via vcpkg). It regularly fails to build for me
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
  third-party/folly/folly/lang/Exception.cpp
  third-party/folly/folly/system/ThreadId.cpp
  third-party/folly/folly/system/AtFork.cpp
  third-party/folly/folly/detail/Futex.cpp
)

add_library(folly_lib STATIC ${folly_sources})

# Not our code. Don't try to lint it.
set_source_files_properties(${folly_sources} PROPERTIES SKIP_LINTING ON)

target_include_directories(
    folly_lib
    SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/folly>"
    ${BOOST_INCLUDE_DIRS}
)

set_property(TARGET folly_lib PROPERTY OUTPUT_NAME folly)

target_compile_features(folly_lib PUBLIC ${jank_cxx_standard})
target_compile_options(folly_lib PUBLIC ${jank_common_compiler_flags} ${jank_aot_compiler_flags} -w)
target_link_options(folly_lib PRIVATE ${jank_linker_flags})

#set_target_properties(folly_lib PROPERTIES LINK_FLAGS_RELEASE "-s")

# Symbol exporting for JIT.
set_target_properties(folly_lib PROPERTIES ENABLE_EXPORTS 1)

target_link_libraries(
  folly_lib PUBLIC
  Boost::preprocessor
)
# ---- libfolly.a ----

# ---- jank executable ----
add_executable(
  jank_exe
  src/cpp/main.cpp
)
add_executable(jank::exe ALIAS jank_exe)

set_property(TARGET jank_exe PROPERTY OUTPUT_NAME jank)

# Symbol exporting for JIT.
set_target_properties(jank_exe PROPERTIES ENABLE_EXPORTS 1)

target_compile_features(jank_exe PRIVATE ${jank_cxx_standard})
target_compile_options(jank_exe PUBLIC ${jank_common_compiler_flags} ${jank_aot_compiler_flags})
target_link_options(jank_exe PRIVATE ${jank_linker_flags})

target_include_directories(jank_exe SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})

target_link_libraries(
  jank_exe PUBLIC
  ${jank_link_whole_start} jank_lib ${jank_link_whole_end}
  ${jank_link_whole_start} nanobench_lib ${jank_link_whole_end}
  folly_lib
  fmt::fmt
  Boost::boost Boost::filesystem Boost::system
)

jank_hook_llvm(jank_exe)

#set_target_properties(jank_exe PROPERTIES LINK_FLAGS_RELEASE "-s")
# ---- jank executable ----

# ---- Tests ----
if(jank_tests)
  add_executable(
    jank_test_exe
    test/cpp/main.cpp
    test/cpp/jank/native_persistent_string.cpp
    test/cpp/jank/read/lex.cpp
    test/cpp/jank/read/parse.cpp
    test/cpp/jank/analyze/box.cpp
    test/cpp/jank/runtime/detail/list_type.cpp
    test/cpp/jank/jit/processor.cpp
  )
  add_executable(jank::test_exe ALIAS jank_test_exe)
  add_dependencies(jank_test_exe jank_lib jank_core_libraries)

  set_property(TARGET jank_test_exe PROPERTY OUTPUT_NAME jank-test)

  target_compile_features(jank_test_exe PRIVATE ${jank_cxx_standard})
  target_compile_options(jank_test_exe PUBLIC ${jank_common_compiler_flags} ${jank_aot_compiler_flags})
  target_compile_options(jank_test_exe PRIVATE -DDOCTEST_CONFIG_SUPER_FAST_ASSERTS)
  target_link_options(jank_test_exe PRIVATE ${jank_linker_flags})

  target_include_directories(jank_test_exe SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})

  find_package(doctest REQUIRED)
  target_link_libraries(
    jank_test_exe PUBLIC
    ${jank_link_whole_start} jank_lib ${jank_link_whole_end}
    ${jank_link_whole_start} nanobench_lib ${jank_link_whole_end}
    folly_lib
    doctest::doctest
    Boost::boost Boost::filesystem
  )

  jank_hook_llvm(jank_test_exe)

  # Symbol exporting for JIT.
  set_target_properties(jank_test_exe PROPERTIES ENABLE_EXPORTS 1)

  add_test(NAME "Test" COMMAND jank_test_exe)
endif()
# ---- Tests ----

# ---- Incremental PCH ----
# jank has two types of pre-compiled headers, since Clang's JIT compiler is picky. The first
# PCH is handled normally via CMake using `target_precompile_headers` and it's used for AOT builts.
# However, once we boot up jank, the first thing we do is load a PCH so that the JIT environment
# can know all of the types and functions within the jank runtime. This second PCH is our
# "incremental" PCH, named that way since it uses Clang's -fincremental-extensions flag.
#
# Here, we set up the incremental PCH to be built whenever the normal PCH is changed or the script
# for building the incremental PCH changes. Any other changes don't matter.

# Build a string of all of our AOT compilation flags so we can share them with our incremental PCH.
set(jank_pch_compile_flags_list ${jank_common_compiler_flags} -w)
list(JOIN jank_pch_compile_flags_list " " jank_pch_compile_flags_str)

set(jank_incremental_pch_flag ${CMAKE_BINARY_DIR}/incremental.pch)
add_custom_command(
  DEPENDS ${CMAKE_BINARY_DIR}/jank ${CMAKE_SOURCE_DIR}/bin/build-pch
  OUTPUT ${jank_incremental_pch_flag}
  COMMAND "${CMAKE_SOURCE_DIR}/bin/build-pch" "${CLANG_INSTALL_PREFIX}" "${CMAKE_SOURCE_DIR}/include" "${jank_incremental_pch_flag}" "${jank_pch_compile_flags_str}" "${jank_lib_includes_str}"
)
add_custom_target(
  jank_incremental_pch
  ALL
  DEPENDS ${jank_incremental_pch_flag}
)
# ---- Incremental PCH ----

# ---- Compiled Clojure libraries ----
# We do a bit of a dance here, to have a custom command generate a file
# which is a then a dependency of a custom target. This is because custom
# targets *always* run, but we only want to compile our libs when they change,
# or when we haven't yet done so.
#
# With this setup, we'll compile when the flag file doesn't exist (i.e. on
# first build or after a clean), when any of the jank sources for these libs
# change, or whenever the jank binary changes.
set(jank_core_libraries_flag ${CMAKE_BINARY_DIR}/classes/core-libraries)
add_custom_command(
  # TODO: Depend on jank's actual PCH.
  DEPENDS ${CMAKE_BINARY_DIR}/jank ${CMAKE_SOURCE_DIR}/src/jank/clojure/core.jank ${jank_incremental_pch_flag}
  OUTPUT ${jank_core_libraries_flag}
  COMMAND ${CMAKE_BINARY_DIR}/jank compile clojure.core
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/classes; touch ${jank_core_libraries_flag}
)
add_custom_target(
  jank_core_libraries
  ALL
  DEPENDS ${jank_core_libraries_flag}
)
# ---- Compiled Clojure libraries ----

# ---- Clang format YAML ----
# We copy this into our binary directory so that jank can use it to format code. It's also
# installed alongside jank, for the same reason.
file(COPY ${CMAKE_SOURCE_DIR}/../.clang-format DESTINATION ${CMAKE_BINARY_DIR})
# ---- Clang format YAML ----

# ---- Install rules ----
if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install.cmake)
endif()
# ---- Install rules ----
