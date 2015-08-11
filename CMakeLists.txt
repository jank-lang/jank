cmake_minimum_required(VERSION 2.8)

project(jank)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

# Shut up the warning about CMake policy CMP0042.
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(STATUS "Found 32bit system")
  set(64_BIT_PLATFORM 0)
else()
  message(STATUS "Found 64bit system")
  set(64_BIT_PLATFORM 1)
endif()

# Submodules
message(STATUS "Updating submodules")
execute_process(
  COMMAND git submodule update --recursive --init
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# C++14
include(cmake/cxx14.cmake)

# Boost
find_package(Boost COMPONENTS system REQUIRED)

# PThread
find_package(Threads REQUIRED)

include_directories(
  include
  test/include
  lib/jtl/include
  lib/jest/include
)

add_library(
  ${PROJECT_NAME}_static STATIC
  src/jank/parse/parse.cpp
  src/jank/parse/cell/stream.cpp

  src/jank/translate/cell/stream.cpp
  src/jank/translate/function/argument/definition.cpp
  src/jank/translate/function/argument/call.cpp
  src/jank/translate/function/argument/resolve_type.cpp
  src/jank/translate/function/return/parse.cpp
  src/jank/translate/function/return/validate.cpp
  src/jank/translate/function/return/deduce.cpp
  src/jank/translate/function/return/make_implicit.cpp
  src/jank/translate/environment/scope.cpp
  src/jank/translate/environment/builtin/type/primitive.cpp
  src/jank/translate/environment/builtin/type/function.cpp
  src/jank/translate/environment/builtin/value/primitive.cpp
  src/jank/translate/environment/special/apply_all.cpp
  src/jank/translate/environment/special/apply_expression.cpp
  src/jank/translate/environment/special/function.cpp
  src/jank/translate/environment/special/binding.cpp
  src/jank/translate/environment/special/return_statement.cpp
  src/jank/translate/environment/special/if_statement.cpp
  src/jank/translate/environment/special/do_statement.cpp
  src/jank/translate/plugin/apply.cpp
  src/jank/translate/plugin/io/print.cpp
  src/jank/translate/plugin/arithmetic/add.cpp
  src/jank/translate/plugin/arithmetic/subtract.cpp
  src/jank/translate/plugin/arithmetic/multiply.cpp
  src/jank/translate/plugin/arithmetic/divide.cpp
  src/jank/translate/plugin/arithmetic/modulo.cpp
  src/jank/translate/plugin/arithmetic/bitwise_and.cpp
  src/jank/translate/plugin/arithmetic/bitwise_or.cpp
  src/jank/translate/plugin/arithmetic/bitwise_xor.cpp
  src/jank/translate/plugin/arithmetic/bitwise_not.cpp
  src/jank/translate/plugin/arithmetic/bitwise_left_shift.cpp
  src/jank/translate/plugin/arithmetic/bitwise_right_shift.cpp
  src/jank/translate/plugin/compare/equal.cpp
  src/jank/translate/plugin/compare/less.cpp
  src/jank/translate/plugin/compare/less_equal.cpp
  src/jank/translate/plugin/compare/greater.cpp
  src/jank/translate/plugin/compare/greater_equal.cpp
  src/jank/translate/plugin/assertion/assertion.cpp

  src/jank/interpret/interpret.cpp
  src/jank/interpret/environment/scope.cpp
  src/jank/interpret/environment/resolve_value.cpp
  src/jank/interpret/detail/function_call.cpp
  src/jank/interpret/detail/native_function_call.cpp
  src/jank/interpret/detail/binding_definition.cpp
  src/jank/interpret/detail/if_statement.cpp
  src/jank/interpret/detail/return_statement.cpp
  src/jank/interpret/detail/do_statement.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_THREAD_LIBS_INIT}
  ${PROJECT_NAME}_static
)

# Enable warnings
set_property(
  TARGET ${PROJECT_NAME} ${PROJECT_NAME}_static
  PROPERTY COMPILE_FLAGS "-Wall -Wextra -Werror -pedantic")

# Testing
add_library(
  ${PROJECT_NAME}_test_common_static STATIC
  test/src/cpp/common/interpret.cpp
  test/src/cpp/common/translate.cpp
)

set(
  ${PROJECT_NAME}_test_libs
  ${CMAKE_THREAD_LIBS_INIT}
  ${PROJECT_NAME}_test_common_static
  ${PROJECT_NAME}_static
)

add_executable(${PROJECT_NAME}_parse_test test/src/cpp/parse/main.cpp)
target_link_libraries(${PROJECT_NAME}_parse_test ${${PROJECT_NAME}_test_libs})

add_executable(
  ${PROJECT_NAME}_translate_function_test
  test/src/cpp/translate/function/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_translate_function_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_translate_binding_test
  test/src/cpp/translate/binding/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_translate_binding_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_translate_plugin_test
  test/src/cpp/translate/plugin/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_translate_plugin_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_translate_if_test
  test/src/cpp/translate/if/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_translate_if_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_translate_do_test
  test/src/cpp/translate/do/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_translate_do_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_interpret_function_test
  test/src/cpp/interpret/function/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_interpret_function_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_interpret_binding_test
  test/src/cpp/interpret/binding/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_interpret_binding_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_interpret_if_test
  test/src/cpp/interpret/if/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_interpret_if_test
  ${${PROJECT_NAME}_test_libs}
)

add_executable(
  ${PROJECT_NAME}_interpret_do_test
  test/src/cpp/interpret/do/main.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_interpret_do_test
  ${${PROJECT_NAME}_test_libs}
)

add_custom_target(
  ${PROJECT_NAME}_test
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_parse_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_translate_function_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_translate_binding_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_translate_plugin_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_translate_if_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_translate_do_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_interpret_function_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_interpret_binding_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_interpret_if_test || true && echo &&
          ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_interpret_do_test
  DEPENDS ${PROJECT_NAME}_parse_test
          ${PROJECT_NAME}_translate_function_test
          ${PROJECT_NAME}_translate_binding_test
          ${PROJECT_NAME}_translate_plugin_test
          ${PROJECT_NAME}_translate_if_test
          ${PROJECT_NAME}_translate_do_test
          ${PROJECT_NAME}_interpret_function_test
          ${PROJECT_NAME}_interpret_binding_test
          ${PROJECT_NAME}_interpret_if_test
          ${PROJECT_NAME}_interpret_do_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# Install locally
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
