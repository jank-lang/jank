set(CMAKE_C_FLAGS_OLD "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})
set(CMAKE_CXX_CLANG_TIDY_OLD ${CMAKE_CXX_CLANG_TIDY})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  set(BUILD_SHARED_LIBS OFF)
  set(CMAKE_CXX_CLANG_TIDY "")
  set(enable_cplusplus ON CACHE BOOL "Enable C++")
  set(build_cord OFF CACHE BOOL "Build cord")
  set(enable_docs OFF CACHE BOOL "Enable docs")
  set(enable_throw_bad_alloc_library OFF CACHE BOOL "Enable C++ gctba library build")

  # BDWGC redefines BUILD_SHARED_LIBS as an option. The old policy behavior here
  # is to reset the value to the new default defined, which is ON. However, we
  # want it to keep the value we've already specified, to force BDWGC to build
  # as a static lib, so we choose the new behavior.
  cmake_policy(SET CMP0077 NEW)

  add_subdirectory(third-party/bdwgc EXCLUDE_FROM_ALL)

  unset(enable_cplusplus)
  unset(build_cord)
  unset(enable_docs)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_OLD}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_OLD}")
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_OLD})
set(CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY_OLD})
