cmake_minimum_required(VERSION 3.21)
project(nrepl_server LANGUAGES CXX)

# Disable FindBoost warning
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost CONFIG REQUIRED)

# Define the shared library for the native component.
add_library(nrepl_server_native SHARED src/cpp/nrepl_server/nrepl_server.cpp)
target_include_directories(nrepl_server_native PUBLIC src/cpp)
target_link_libraries(nrepl_server_native PRIVATE Boost::boost)

# Define a dummy interface target for the jank script which we will use to
# collect include directories, link directories, and link libraries to be passed
# to jank.
add_library(nrepl_server INTERFACE)
target_link_libraries(nrepl_server INTERFACE nrepl_server_native)
target_link_directories(nrepl_server INTERFACE "$<TARGET_LINKER_FILE_DIR:nrepl_server_native>")

# -I, -L, and -l flags respectively.
set(JANK_WRAPPER_COMPILE_DEFINITIONS "$<TARGET_PROPERTY:nrepl_server,INTERFACE_COMPILE_DEFINITIONS>")
set(JANK_WRAPPER_INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:nrepl_server,INTERFACE_INCLUDE_DIRECTORIES>")
set(JANK_WRAPPER_LINK_DIRECTORIES    "$<TARGET_PROPERTY:nrepl_server,INTERFACE_LINK_DIRECTORIES>")
set(JANK_WRAPPER_LINK_LIBRARIES      "$<TARGET_PROPERTY:nrepl_server,INTERFACE_LINK_LIBRARIES>")

# Create a jank-wrapper script which passes all these compiler options under the
# hood.
set(JANK_WRAPPER_CMD "\
#!/usr/bin/env bash
set -eu
jank $<$<BOOL:${JANK_WRAPPER_COMPILE_DEFINITIONS}>:-D$<JOIN:${JANK_WRAPPER_COMPILE_DEFINITIONS}, -D>> \
     $<$<BOOL:${JANK_WRAPPER_INCLUDE_DIRECTORIES}>:-I$<JOIN:${JANK_WRAPPER_INCLUDE_DIRECTORIES}, -I>> \
     $<$<BOOL:${JANK_WRAPPER_LINK_DIRECTORIES}>:-L$<JOIN:${JANK_WRAPPER_LINK_DIRECTORIES}, -L>> \
     $<$<BOOL:${JANK_WRAPPER_LINK_LIBRARIES}>:-l$<JOIN:${JANK_WRAPPER_LINK_LIBRARIES}, -l>> \
     --module-path ${PROJECT_SOURCE_DIR}/src/jank \
     $\{@:1\}")
file(
  GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/jank-wrapper"
  CONTENT "${JANK_WRAPPER_CMD}"
  FILE_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)

# TODO: handle install
