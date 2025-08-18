cmake_minimum_required(VERSION 3.21)
project(nrepl_server LANGUAGES CXX)

find_package(Boost REQUIRED system)

add_library(nrepl_server SHARED
  src/cpp/nrepl_server/nrepl_server.cpp)
target_include_directories(nrepl_server PRIVATE
  src/cpp/nrepl_server)
target_link_libraries(nrepl_server PUBLIC Boost::system)
