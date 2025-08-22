cmake_minimum_required(VERSION 3.21)
project(nrepl_server LANGUAGES CXX)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS system)

add_library(nrepl_server SHARED
  src/cpp/nrepl_server/nrepl_server.cpp)
target_include_directories(nrepl_server PRIVATE
  src/cpp/nrepl_server)
target_link_libraries(nrepl_server PRIVATE Boost::system)

set(SERVER_EXEC_CMD "\
jank -I${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/ \
     -L${CMAKE_CURRENT_BINARY_DIR} -lnrepl_server \
     --module-path src/jank/ \
     run-main nrepl-server.core $\{@:1\} \
")
file(
  GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/jank-nrepl-server"
  CONTENT "${SERVER_EXEC_CMD}"
  FILE_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)
