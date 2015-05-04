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

# C++14
include(cmake/cxx14.cmake)

# Boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system REQUIRED)

# PThread
find_package(Threads REQUIRED)

include_directories(
  include
  test/include
  lib/jtl/include
  lib/jest/include
)

add_executable(${PROJECT_NAME}
               src/main.cpp
	             src/jank/parse/parse.cpp
	             src/jank/translate/cell/stream.cpp
	             src/jank/translate/function/argument/definition.cpp
               src/jank/translate/function/argument/call.cpp
               src/jank/translate/environment/scope.cpp
               src/jank/translate/environment/special/func.cpp
)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

# Enable warnings
set_property(
  TARGET ${PROJECT_NAME}
  PROPERTY COMPILE_FLAGS "-Wall -Wextra -Werror -pedantic")

# Testing
add_executable(${PROJECT_NAME}_function_test
               test/src/cpp/function/main.cpp
	             src/jank/parse/parse.cpp
)
add_custom_target(${PROJECT_NAME}_test
  COMMAND ${PROJECT_NAME}_function_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME}_function_test)

# Install locally
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
