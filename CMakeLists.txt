cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
    FATAL_ERROR
    "In-source builds are not supported. "
    "Please read README.md before trying to build this project. "
    "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()

# jank can only be built with the corresponding Clang that Cling will be using,
# since the JIT compiled code needs to match 1:1 with the AOT compiled runtime.
# We just pull the Clang compiler right out of the Cling build dir.
option(jank_cling_build_dir "Location of Cling source, if built locally" "")
cmake_path(ABSOLUTE_PATH jank_cling_build_dir OUTPUT_VARIABLE jank_cling_build_dir_absolute)
set(CMAKE_C_COMPILER "${jank_cling_build_dir_absolute}/bin/clang")
set(CMAKE_CXX_COMPILER "${jank_cling_build_dir_absolute}/bin/clang++")

include(cmake/vcpkg.cmake)

# LTO is explicitly not used, since it leads to runtime symbol lookup issues with the JIT.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)

set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/vcpkg-triplet")
set(VCPKG_TARGET_TRIPLET "x64-clang-static")

project(
    jank
    VERSION 0.1
    DESCRIPTION "A Clojure dialect on LLVM with gradual typing, a native runtime, and C++ interop."
    HOMEPAGE_URL "https://jank-lang.org/"
    # C is needed here, due to a clang issue:
    # https://stackoverflow.com/questions/71740678/cmake-error-in-findterminfo-with-clang-15-on-macos
    LANGUAGES C CXX
)

set(jank_cxx_standard cxx_std_20)

set(jank_required_compiler_id "Clang")
set(jank_required_compiler_version "13.0.0")

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

if(
  NOT CMAKE_CXX_COMPILER_ID STREQUAL jank_required_compiler_id
  OR
  NOT CMAKE_CXX_COMPILER_VERSION STREQUAL jank_required_compiler_version
)
  message(
    FATAL_ERROR
    "${jank_required_compiler_id} ${jank_required_compiler_version} is required to build jank, "
    "but ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} was found. "
    "Please follow the README.md for build instructions."
  )
endif()

set(
  jank_compiler_flags
  -Wall -Wextra -Wpedantic
  -Wfloat-equal -Wuninitialized -Wswitch-enum -Wnon-virtual-dtor
  -Wold-style-cast -Wno-gnu-case-range
  -DJANK_CLING_BUILD_DIR="${jank_cling_build_dir_absolute}"
  -DIMMER_HAS_LIBGC=1 -DHAVE_CXX14=1
  $<$<CONFIG:Release>:-O3>
  #-stdlib=libc++
)
set(
  jank_linker_flags
  -L${jank_cling_build_dir}/build-compiler-rt/lib/linux
  #-stdlib=libc++ -lc++abi
  )
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND jank_compiler_flags -Werror)
endif()

include(cmake/variables.cmake)
include(cmake/coverage.cmake)
include(cmake/analysis.cmake)

# Enable easier tooling during development.
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# ---- libjankcling.so ----
# Cling, Clang, and LLVM are huge. About 1.5G unstripped and 150M stripped.
# This takes a long time to link, so doing it as part of the jank lib or
# executable hurts iteration time on hot reloading tests and, in general,
# compiling more than once. So this is just a lib to suck all of that up.

# CMake doesn't support libraries/executables without sources, but we're just
# combining libraries here, so we make a dummy source just to please it.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/null.cpp" "")

add_library(jank_cling_lib SHARED "${CMAKE_CURRENT_BINARY_DIR}/null.cpp")

set_property(TARGET jank_cling_lib PROPERTY OUTPUT_NAME jankcling)

target_compile_features(jank_cling_lib PUBLIC ${jank_cxx_standard})

find_package(LLVM REQUIRED CONFIG HINTS "${jank_cling_build_dir}/lib/cmake/llvm" NO_DEFAULT_PATH)
find_package(Clang REQUIRED CONFIG HINTS "${jank_cling_build_dir}/lib/cmake/clang" NO_DEFAULT_PATH)
find_package(
  Cling REQUIRED
  HINTS "${jank_cling_build_dir}/tools/cling/lib/cmake/cling"
        "${jank_cling_build_dir}/lib/cmake/cling"
  NO_DEFAULT_PATH
)

list(APPEND CLING_INCLUDE_DIRS ${jank_cling_build_dir}/build-compiler-rt/include)

target_include_directories(jank_cling_lib SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_cling_lib SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_cling_lib SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_directories(jank_cling_lib PRIVATE ${LLVM_LIBRARY_DIRS})
target_link_libraries(jank_cling_lib PRIVATE ${jank_link_whole_start} clingInterpreter ${jank_link_whole_end})

set_target_properties(jank_cling_lib PROPERTIES LINK_FLAGS -s)
# ---- libjankcling.so ----

# ---- libjank.a ----
# The sources for jank are used for both the compiler and its tests, so
# they're just pulled into a static library. It's important that this
# remains a static lib, since these symbols need to be accessible in the
# compiler's runtime by the JIT compiler.

add_library(
  jank_lib STATIC
  src/cpp/jank/native_box.cpp
  src/cpp/jank/util/mapped_file.cpp
  src/cpp/jank/util/scope_exit.cpp
  src/cpp/jank/read/lex.cpp
  src/cpp/jank/read/parse.cpp
  src/cpp/jank/runtime/module/loader.cpp
  src/cpp/jank/runtime/util.cpp
  src/cpp/jank/runtime/seq.cpp
  src/cpp/jank/runtime/object.cpp
  src/cpp/jank/runtime/detail/object_util.cpp
  src/cpp/jank/runtime/context.cpp
  src/cpp/jank/runtime/ns.cpp
  src/cpp/jank/runtime/var.cpp
  src/cpp/jank/runtime/obj/nil.cpp
  src/cpp/jank/runtime/obj/number.cpp
  src/cpp/jank/runtime/obj/native_function_wrapper.cpp
  src/cpp/jank/runtime/obj/jit_function.cpp
  src/cpp/jank/runtime/obj/symbol.cpp
  src/cpp/jank/runtime/obj/keyword.cpp
  src/cpp/jank/runtime/obj/list.cpp
  src/cpp/jank/runtime/obj/vector.cpp
  src/cpp/jank/runtime/obj/map.cpp
  src/cpp/jank/runtime/obj/set.cpp
  src/cpp/jank/runtime/obj/string.cpp
  src/cpp/jank/runtime/obj/cons.cpp
  src/cpp/jank/runtime/obj/range.cpp
  src/cpp/jank/runtime/obj/iterator.cpp
  src/cpp/jank/runtime/obj/persistent_vector_sequence.cpp
  src/cpp/jank/runtime/obj/persistent_map_sequence.cpp
  src/cpp/jank/runtime/obj/native_array_sequence.cpp
  src/cpp/jank/runtime/obj/native_vector_sequence.cpp
  src/cpp/jank/runtime/behavior/callable.cpp
  src/cpp/jank/runtime/behavior/metadatable.cpp
  src/cpp/jank/runtime/math.cpp
  src/cpp/jank/analyze/processor.cpp
  src/cpp/jank/analyze/local_frame.cpp
  src/cpp/jank/analyze/step/force_boxed.cpp
  src/cpp/jank/evaluate.cpp
  src/cpp/jank/codegen/processor.cpp
  src/cpp/jank/jit/processor.cpp
)

target_include_directories(
    jank_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/cpp>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/nanobench/include>"
)
target_precompile_headers(jank_lib PUBLIC ${PROJECT_SOURCE_DIR}/include/cpp/jank/prelude.hpp)

set_property(TARGET jank_lib PROPERTY OUTPUT_NAME jank)

target_compile_features(jank_lib PUBLIC ${jank_cxx_standard})
target_compile_options(jank_lib PUBLIC ${jank_compiler_flags})
list(JOIN jank_compiler_flags " " jank_compiler_flags_str)
string(REPLACE "\"" "\\\"" jank_compiler_flags_str ${jank_compiler_flags_str})
target_compile_options(jank_lib PUBLIC -DJANK_COMPILER_FLAGS="${jank_compiler_flags_str}")
target_link_options(jank_lib PRIVATE ${jank_linker_flags})

find_package(Immer CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(folly CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(BDWgc CONFIG REQUIRED)
find_package(libzippp CONFIG REQUIRED)

target_include_directories(jank_lib SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_directories(jank_lib PRIVATE ${jank_cling_build_dir}/build-compiler-rt/lib/linux)
target_link_libraries(
  jank_lib PRIVATE
  jank_cling_lib immer fmt::fmt
  Folly::folly Folly::folly_deps
  BDWgc::gc BDWgc::cord BDWgc::gccpp BDWgc::gctba
  libzippp::libzippp
)

set_target_properties(jank_lib PROPERTIES LINK_FLAGS_RELEASE "-s")

# Symbol exporting for JIT.
set_target_properties(jank_lib PROPERTIES ENABLE_EXPORTS 1)
# ---- libjank.a ----

# ---- libnanobench.a ----
# nanobench uses a single header for both .hpp and .cpp inclusion, based on
# whether a define has been set. This doesn't work with jank's pre-compiled
# headers, so I've manually split the two up and included them here.

add_library(
  nanobench_lib STATIC
  third-party/nanobench/src/nanobench.cpp
)

target_include_directories(
    nanobench_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/nanobench/include>"
)

set_property(TARGET nanobench_lib PROPERTY OUTPUT_NAME nanobench)

target_compile_features(nanobench_lib PUBLIC ${jank_cxx_standard})

set_target_properties(nanobench_lib PROPERTIES LINK_FLAGS_RELEASE "-s")

# Symbol exporting for JIT.
set_target_properties(nanobench_lib PROPERTIES ENABLE_EXPORTS 1)
# ---- libnanobench.a ----

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
target_compile_options(jank_exe PUBLIC ${jank_compiler_flags})
target_link_options(jank_exe PRIVATE ${jank_linker_flags})

target_include_directories(jank_exe SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})
target_include_directories(jank_exe SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_exe SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_exe SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_libraries(
  jank_exe PUBLIC
  ${jank_link_whole_start} jank_lib ${jank_link_whole_end}
  ${jank_link_whole_start} nanobench_lib ${jank_link_whole_end}
  Boost::boost
)
set_target_properties(jank_exe PROPERTIES LINK_FLAGS_RELEASE -s)
# ---- jank executable ----

# ---- Tests ----
if(jank_tests)
  add_executable(
    jank_test_exe
    test/cpp/main.cpp
    test/cpp/jank/read/lex.cpp
    test/cpp/jank/read/parse.cpp
    test/cpp/jank/analyze/box.cpp
    test/cpp/jank/runtime/detail/list_type.cpp
    test/cpp/jank/runtime/context.cpp
    test/cpp/jank/jit/processor.cpp
  )
  add_executable(jank::test_exe ALIAS jank_test_exe)

  set_property(TARGET jank_test_exe PROPERTY OUTPUT_NAME jank-test)

  target_compile_features(jank_test_exe PRIVATE ${jank_cxx_standard})
  target_compile_options(jank_test_exe PUBLIC ${jank_compiler_flags})
  target_compile_options(jank_test_exe PRIVATE -DDOCTEST_CONFIG_SUPER_FAST_ASSERTS)
  target_link_options(jank_test_exe PRIVATE ${jank_linker_flags})

  target_include_directories(jank_test_exe SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

  find_package(doctest REQUIRED)
  target_link_libraries(
    jank_test_exe PUBLIC
    ${jank_link_whole_start} jank_lib ${jank_link_whole_end}
    ${jank_link_whole_start} nanobench_lib ${jank_link_whole_end}
    Boost::boost
    doctest::doctest
  )

  # Symbol exporting for JIT.
  set_target_properties(jank_test_exe PROPERTIES ENABLE_EXPORTS 1)

  add_test(NAME "Test" COMMAND jank_test_exe)
endif()
# ---- Tests ----

# ---- Install rules ----
if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install.cmake)
endif()
# ---- Install rules ----
