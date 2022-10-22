cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
      FATAL_ERROR
      "In-source builds are not supported. "
      "Please read README.md before trying to build this project. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()

include(cmake/vcpkg.cmake)

# LTO is explicitly not used, since it leads to runtime symbol lookup issues with the JIT.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)

project(
    jank
    VERSION 0.1
    DESCRIPTION "A Clojure dialect on LLVM with gradual typing, a native runtime, and C++ interop."
    HOMEPAGE_URL "https://jank-lang.org/"
    # C is needed here, due to a clang issue:
    # https://stackoverflow.com/questions/71740678/cmake-error-in-findterminfo-with-clang-15-on-macos
    LANGUAGES C CXX
)

include(cmake/variables.cmake)
include(cmake/coverage.cmake)
include(cmake/analysis.cmake)

if(MSVC)
  set(jank_compiler_flags /W4)
  set(jank_linker_flags "")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND jank_compiler_flags /WX)
  endif()
else()
  set(
    jank_compiler_flags
    -Wall -Wextra -Wpedantic
    -Wfloat-equal -Wuninitialized -Wswitch-enum -Wnon-virtual-dtor
    -Wold-style-cast -Wno-gnu-case-range
  )
  set(jank_linker_flags "")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND jank_compiler_flags -Werror)
  endif()
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

option(jank_build_cling "Whether or not to automatically build Cling" OFF)
option(jank_cling_build_dir "Location of Cling source, if built locally" "")
option(jank_llvm_root "Path to LLVM's resource dir" "")

if(jank_build_cling)
  message(STATUS "Building Cling/Clang/LLVM locally")
  execute_process(
    COMMAND ${PROJECT_SOURCE_DIR}/bin/build-cling
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_ERROR_IS_FATAL ANY
  )
  set(jank_cling_build_dir "${CMAKE_CURRENT_BINARY_DIR}/cling-build/builddir")
  set(jank_llvm_root "${CMAKE_CURRENT_BINARY_DIR}/cling-build/builddir")
endif()

find_package(LLVM REQUIRED HINTS "${jank_cling_build_dir}/lib/cmake/llvm")
find_package(Clang REQUIRED CONFIG HINTS "${jank_cling_build_dir}/lib/cmake/clang" NO_DEFAULT_PATH)
find_package(
  Cling REQUIRED
  HINTS "${jank_cling_build_dir}/tools/cling/lib/cmake/cling"
        "${jank_cling_build_dir}/lib/cmake/cling"
)

# This can be set manually if we're using an external Cling, but, if it's not set,
# we can assume we're using our own Cling and default to a sane value.
if(NOT jank_build_cling AND NOT jank_llvm_root)
  set(jank_llvm_root ${LLVM_BINARY_DIR})
endif()

# Cling 0.8 doesn't install sanely, compared to master, so we need to set some things up ourselves.
# See bin/build-cling for more details.
if(jank_cling_build_dir)
  list(APPEND CLING_INCLUDE_DIRS "${jank_cling_build_dir}/../cling-src/tools/cling/include")
endif()

target_include_directories(jank_cling_lib SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_cling_lib SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_cling_lib SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_directories(jank_cling_lib SYSTEM PRIVATE ${LLVM_LIBRARY_DIRS})

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  target_link_libraries(jank_cling_lib PRIVATE -Wl,-force_load, clingInterpreter)
elseif(MSVC)
  target_link_libraries(jank_cling_lib PRIVATE -WHOLEARCHIVE: clingInterpreter)
else()
  target_link_libraries(jank_cling_lib PRIVATE -Wl,--whole-archive clingInterpreter -Wl,--no-whole-archive)
endif()

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
  src/cpp/jank/analyze/processor.cpp
  src/cpp/jank/evaluate/context.cpp
  src/cpp/jank/codegen/processor.cpp
  src/cpp/jank/jit/processor.cpp
)

target_include_directories(
    jank_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/cpp>"
)

set_property(TARGET jank_lib PROPERTY OUTPUT_NAME jank)

target_compile_features(jank_lib PUBLIC cxx_std_17)
target_compile_options(jank_lib PRIVATE ${jank_compiler_flags})
target_compile_options(jank_lib PUBLIC -DLLVMDIR="${jank_llvm_root}")
target_link_options(jank_lib PRIVATE ${jank_linker_flags})

find_package(Boost REQUIRED)
find_package(folly REQUIRED)
find_package(Immer REQUIRED)
find_package(magic_enum REQUIRED)

target_include_directories(jank_lib SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(jank_lib SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_libraries(jank_lib PRIVATE jank_cling_lib immer Folly::folly Folly::folly_deps lzma)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_link_libraries(jank_lib PRIVATE unwind)
endif()

set_target_properties(jank_lib PROPERTIES LINK_FLAGS_RELEASE -s)
# ---- libjank.a ----

# ---- jank executable ----
add_executable(jank_exe src/cpp/main.cpp)
add_executable(jank::exe ALIAS jank_exe)

set_property(TARGET jank_exe PROPERTY OUTPUT_NAME jank)

# Symbol exporting for JIT.
set_target_properties(jank_exe PROPERTIES ENABLE_EXPORTS 1)
if(MSVC)
  set_target_properties(jank_exe PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif()

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
    test/cpp/jank/analyze/expr/let.cpp
    test/cpp/jank/analyze/expr/primitive_literal.cpp
    test/cpp/jank/analyze/expr/map.cpp
    test/cpp/jank/evaluate/context.cpp
    test/cpp/jank/jit/processor.cpp
  )
  add_executable(jank::test_exe ALIAS jank_test_exe)

  set_property(TARGET jank_test_exe PROPERTY OUTPUT_NAME jank-test)

  target_compile_features(jank_test_exe PRIVATE cxx_std_17)
  target_compile_options(jank_test_exe PRIVATE ${jank_compiler_flags})
  target_link_options(jank_test_exe PRIVATE ${jank_linker_flags})

  target_include_directories(jank_test_exe SYSTEM PRIVATE ${BOOST_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${CLING_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
  target_include_directories(jank_test_exe SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

  find_package(doctest REQUIRED)
  target_link_libraries(jank_test_exe PRIVATE jank_lib doctest::doctest)

# Symbol exporting for JIT.
  set_target_properties(jank_test_exe PROPERTIES ENABLE_EXPORTS 1)
  if(MSVC)
    set_target_properties(jank_test_exe PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
  endif()

  add_test(NAME "Test" COMMAND jank_test_exe)
endif()
# ---- Tests ----

# ---- Install rules ----
if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install.cmake)
endif()
# ---- Install rules ----
