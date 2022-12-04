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

# TODO: Disable PCH during analysis builds

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

set(jank_required_compiler_id "Clang")
set(jank_required_compiler_version "9.0.1")

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

include(cmake/variables.cmake)
include(cmake/coverage.cmake)
include(cmake/analysis.cmake)

set(
  jank_compiler_flags
  -Wall -Wextra -Wpedantic
  -Wfloat-equal -Wuninitialized -Wswitch-enum -Wnon-virtual-dtor
  -Wold-style-cast -Wno-gnu-case-range
  #-stdlib=libc++
)
set(jank_linker_flags "" #-stdlib=libc++ -lc++abi
  )
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND jank_compiler_flags -Werror)
endif()

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

target_compile_features(jank_cling_lib PUBLIC cxx_std_17)

find_package(LLVM REQUIRED CONFIG HINTS "${jank_cling_build_dir}/lib/cmake/llvm" NO_DEFAULT_PATH)
find_package(Clang REQUIRED CONFIG HINTS "${jank_cling_build_dir}/lib/cmake/clang" NO_DEFAULT_PATH)
find_package(
  Cling REQUIRED
  HINTS "${jank_cling_build_dir}/tools/cling/lib/cmake/cling"
        "${jank_cling_build_dir}/lib/cmake/cling"
  NO_DEFAULT_PATH
)

option(jank_enable_pch "Enable precompiled header creation" ON)
set(jank_llvm_root ${LLVM_BINARY_DIR})

target_include_directories(jank_cling_lib SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_cling_lib SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_cling_lib SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_directories(jank_cling_lib SYSTEM PRIVATE ${LLVM_LIBRARY_DIRS})
target_link_libraries(jank_cling_lib PRIVATE -Wl,--whole-archive clingInterpreter -Wl,--no-whole-archive)

set_target_properties(jank_cling_lib PROPERTIES LINK_FLAGS -s)
# ---- libjankcling.so ----

# ---- libjank.a ----
# The sources for jank are used for both the compiler and its tests, so
# they're just pulled into a static library. It's important that this
# remains a static lib, since these symbols need to be accessible in the
# compiler's runtime by the JIT compiler.

add_library(
  jank_lib STATIC
  src/cpp/jank/util/mapped_file.cpp
  src/cpp/jank/read/lex.cpp
  src/cpp/jank/read/parse.cpp
  src/cpp/jank/runtime/util.cpp
  src/cpp/jank/runtime/seq.cpp
  src/cpp/jank/runtime/memory_pool.cpp
  src/cpp/jank/runtime/object.cpp
  src/cpp/jank/runtime/context.cpp
  src/cpp/jank/runtime/ns.cpp
  src/cpp/jank/runtime/var.cpp
  src/cpp/jank/runtime/obj/number.cpp
  src/cpp/jank/runtime/obj/function.cpp
  src/cpp/jank/runtime/obj/symbol.cpp
  src/cpp/jank/runtime/obj/keyword.cpp
  src/cpp/jank/runtime/obj/list.cpp
  src/cpp/jank/runtime/obj/vector.cpp
  src/cpp/jank/runtime/obj/map.cpp
  src/cpp/jank/runtime/obj/set.cpp
  src/cpp/jank/runtime/obj/string.cpp
  src/cpp/jank/runtime/behavior/callable.cpp
  src/cpp/jank/runtime/behavior/metadatable.cpp
  src/cpp/jank/analyze/processor.cpp
  src/cpp/jank/analyze/local_frame.cpp
  src/cpp/jank/codegen/processor.cpp
  src/cpp/jank/jit/processor.cpp
)

target_include_directories(
    jank_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/cpp>"
)
if(jank_enable_pch)
  target_precompile_headers(jank_lib PUBLIC ${PROJECT_SOURCE_DIR}/include/cpp/jank/prelude.hpp)
endif()

set_property(TARGET jank_lib PROPERTY OUTPUT_NAME jank)

target_compile_features(jank_lib PUBLIC cxx_std_17)
target_compile_options(jank_lib PRIVATE ${jank_compiler_flags})
target_compile_options(jank_lib PUBLIC -DLLVMDIR="${jank_llvm_root}")
target_link_options(jank_lib PRIVATE ${jank_linker_flags})

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Immer CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(LIBGUARDED_INCLUDE_DIRS "libguarded/shared_guarded.hpp")
target_compile_options(jank_lib PUBLIC -DHAVE_CXX14=1)

target_include_directories(jank_lib SYSTEM PRIVATE ${LIBGUARDED_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_libraries(jank_lib PRIVATE jank_cling_lib immer fmt::fmt Boost::boost Boost::filesystem)

set_target_properties(jank_lib PROPERTIES LINK_FLAGS_RELEASE "-s")

# Symbol exporting for JIT.
set_target_properties(jank_lib PROPERTIES ENABLE_EXPORTS 1)
# ---- libjank.a ----

# ---- jank executable ----
add_executable(jank_exe src/cpp/main.cpp)
add_executable(jank::exe ALIAS jank_exe)

set_property(TARGET jank_exe PROPERTY OUTPUT_NAME jank)

# Symbol exporting for JIT.
set_target_properties(jank_exe PROPERTIES ENABLE_EXPORTS 1)

target_compile_features(jank_exe PRIVATE cxx_std_17)
target_compile_options(jank_exe PUBLIC -DLLVMDIR="${jank_llvm_root}")
target_compile_options(jank_exe PRIVATE ${jank_compiler_flags})
target_link_options(jank_exe PRIVATE ${jank_linker_flags})

target_include_directories(jank_exe SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})
target_include_directories(jank_exe SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_exe SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_exe SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_libraries(jank_exe PRIVATE jank_lib Boost::boost)
set_target_properties(jank_exe PROPERTIES LINK_FLAGS_RELEASE -s)
# ---- jank executable ----

# ---- Tests ----
if(jank_tests)
  add_executable(
    jank_test_exe
    test/cpp/main.cpp
    test/cpp/jank/read/lex.cpp
    test/cpp/jank/read/parse.cpp
    test/cpp/jank/runtime/detail/list_type.cpp
    test/cpp/jank/runtime/context.cpp
    test/cpp/jank/analyze/expr/def.cpp
    test/cpp/jank/analyze/expr/var_deref.cpp
    test/cpp/jank/analyze/expr/function.cpp
    test/cpp/jank/analyze/expr/call.cpp
    test/cpp/jank/analyze/expr/let.cpp
    test/cpp/jank/analyze/expr/if.cpp
    test/cpp/jank/analyze/expr/primitive_literal.cpp
    test/cpp/jank/analyze/expr/map.cpp
    test/cpp/jank/analyze/expr/native_raw.cpp
    test/cpp/jank/jit/processor.cpp
  )
  add_executable(jank::test_exe ALIAS jank_test_exe)

  set_property(TARGET jank_test_exe PROPERTY OUTPUT_NAME jank-test)

  target_compile_features(jank_test_exe PRIVATE cxx_std_17)
  target_compile_options(jank_test_exe PRIVATE ${jank_compiler_flags})
  target_compile_options(jank_test_exe PRIVATE -DDOCTEST_CONFIG_SUPER_FAST_ASSERTS)
  target_link_options(jank_test_exe PRIVATE ${jank_linker_flags})

  target_include_directories(jank_test_exe SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

  find_package(doctest REQUIRED)
  target_link_libraries(jank_test_exe PRIVATE jank_lib Boost::boost doctest::doctest)
  if(jank_enable_pch)
    target_precompile_headers(jank_test_exe REUSE_FROM jank_lib)
  endif()

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
