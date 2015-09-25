cmake_minimum_required(VERSION 2.8)

project(jank)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE DEBUG)
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
set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS system regex REQUIRED)

# PThread
find_package(Threads REQUIRED)

include_directories(
  include
  test/include
  lib/jtl/include
  lib/jest/include
  ${Boost_INCLUDE_DIRS}
)

add_library(
  ${PROJECT_NAME} STATIC
  src/jank/parse/parse.cpp
  src/jank/parse/cell/stream.cpp

  src/jank/translate/cell/stream.cpp
  src/jank/translate/type/generic/extract.cpp
  src/jank/translate/type/generic/parse.cpp
  src/jank/translate/type/generic/verify.cpp
  src/jank/translate/function/argument/definition.cpp
  src/jank/translate/function/argument/call.cpp
  src/jank/translate/function/argument/resolve_type.cpp
  src/jank/translate/function/return/parse.cpp
  src/jank/translate/function/return/add_implicit_returns.cpp
  src/jank/translate/function/return/deduce.cpp
  src/jank/translate/function/return/make_implicit.cpp
  src/jank/translate/function/match_indirect.cpp
  src/jank/translate/environment/scope.cpp
  src/jank/translate/environment/builtin/type/primitive.cpp
  src/jank/translate/environment/builtin/type/function.cpp
  src/jank/translate/environment/builtin/type/list.cpp
  src/jank/translate/environment/builtin/type/normalize.cpp
  src/jank/translate/environment/builtin/value/primitive.cpp
  src/jank/translate/environment/special/apply_all.cpp
  src/jank/translate/environment/special/apply_expression.cpp
  src/jank/translate/environment/special/apply_definition.cpp
  src/jank/translate/environment/special/function.cpp
  src/jank/translate/environment/special/lambda.cpp
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
  src/jank/translate/plugin/collection/list/cons.cpp
  src/jank/translate/plugin/collection/list/first.cpp
  src/jank/translate/plugin/collection/list/rest.cpp
  src/jank/translate/plugin/collection/list/count.cpp
  src/jank/translate/plugin/collection/list/list.cpp

  src/jank/interpret/interpret.cpp
  src/jank/interpret/cell/stream.cpp
  src/jank/interpret/environment/scope.cpp
  src/jank/interpret/environment/resolve_value.cpp
  src/jank/interpret/detail/function_call.cpp
  src/jank/interpret/detail/native_function_call.cpp
  src/jank/interpret/detail/indirect_function_call.cpp
  src/jank/interpret/detail/binding_definition.cpp
  src/jank/interpret/detail/if_statement.cpp
  src/jank/interpret/detail/return_statement.cpp
  src/jank/interpret/detail/do_statement.cpp
  src/jank/interpret/plugin/apply.cpp
  src/jank/interpret/plugin/io/print.cpp
  src/jank/interpret/plugin/arithmetic/add.cpp
  src/jank/interpret/plugin/arithmetic/subtract.cpp
  src/jank/interpret/plugin/arithmetic/multiply.cpp
  src/jank/interpret/plugin/arithmetic/divide.cpp
  src/jank/interpret/plugin/arithmetic/modulo.cpp
  src/jank/interpret/plugin/arithmetic/bitwise_and.cpp
  src/jank/interpret/plugin/arithmetic/bitwise_or.cpp
  src/jank/interpret/plugin/arithmetic/bitwise_xor.cpp
  src/jank/interpret/plugin/arithmetic/bitwise_not.cpp
  src/jank/interpret/plugin/arithmetic/bitwise_left_shift.cpp
  src/jank/interpret/plugin/arithmetic/bitwise_right_shift.cpp
  src/jank/interpret/plugin/compare/equal.cpp
  src/jank/interpret/plugin/compare/less.cpp
  src/jank/interpret/plugin/compare/less_equal.cpp
  src/jank/interpret/plugin/compare/greater.cpp
  src/jank/interpret/plugin/compare/greater_equal.cpp
  src/jank/interpret/plugin/assertion/assertion.cpp
  src/jank/interpret/plugin/collection/list/cons.cpp
  src/jank/interpret/plugin/collection/list/first.cpp
  src/jank/interpret/plugin/collection/list/rest.cpp
  src/jank/interpret/plugin/collection/list/count.cpp
  src/jank/interpret/plugin/collection/list/list.cpp
)

target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
)

add_executable(${PROJECT_NAME}-translate src/jank/translate/main.cpp)

target_link_libraries(
  ${PROJECT_NAME}-translate
  ${CMAKE_THREAD_LIBS_INIT}
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
)

# Enable warnings
set_property(
  TARGET ${PROJECT_NAME}-translate ${PROJECT_NAME}
  PROPERTY COMPILE_FLAGS "-Wall -Wextra -Werror -pedantic")

# Testing
add_library(
  ${PROJECT_NAME}-test-common STATIC
  test/src/cpp/common/interpret.cpp
  test/src/cpp/common/translate.cpp
)

set(
  ${PROJECT_NAME}-test-libs
  ${CMAKE_THREAD_LIBS_INIT}
  ${PROJECT_NAME}-test-common
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
)

# Takes pairs of [test name, test source]
function(make_tests)
  set(TEST_NAME)
  set(TEST_NAMES_LIST)
  set(TEST_COMMAND)
  foreach(ELEMENT ${ARGV})
    if(TEST_NAME)
      add_executable(${TEST_NAME} ${ELEMENT})
      target_link_libraries(${TEST_NAME} ${${PROJECT_NAME}-test-libs})
      set(TEST_NAME)
    else()
      set(TEST_NAME "${ELEMENT}")
      list(APPEND TEST_NAMES_LIST ${TEST_NAME})
      list(APPEND
        TEST_COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME} || true && echo &&
      )
    endif()
  endforeach()

  add_custom_target(
    ${PROJECT_NAME}-test
    COMMAND ${TEST_COMMAND} true
    DEPENDS ${TEST_NAMES_LIST}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )
endfunction()

make_tests(
  ${PROJECT_NAME}-parse-test
  test/src/cpp/parse/main.cpp

  ${PROJECT_NAME}-translate-function-test
  test/src/cpp/translate/function/main.cpp

  ${PROJECT_NAME}-translate-binding-test
  test/src/cpp/translate/binding/main.cpp

  ${PROJECT_NAME}-translate-plugin-test
  test/src/cpp/translate/plugin/main.cpp

  ${PROJECT_NAME}-translate-if-test
  test/src/cpp/translate/if/main.cpp

  ${PROJECT_NAME}-translate-do-test
  test/src/cpp/translate/do/main.cpp

  ${PROJECT_NAME}-translate-lambda-test
  test/src/cpp/translate/lambda/main.cpp

  ${PROJECT_NAME}-interpret-function-test
  test/src/cpp/interpret/function/main.cpp

  ${PROJECT_NAME}-interpret-binding-test
  test/src/cpp/interpret/binding/main.cpp

  ${PROJECT_NAME}-interpret-if-test
  test/src/cpp/interpret/if/main.cpp

  ${PROJECT_NAME}-interpret-do-test
  test/src/cpp/interpret/do/main.cpp

  ${PROJECT_NAME}-interpret-lambda-test
  test/src/cpp/interpret/lambda/main.cpp
)

# Install locally
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})
install(TARGETS ${PROJECT_NAME}-translate DESTINATION bin)
