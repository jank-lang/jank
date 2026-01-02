set(CMAKE_C_FLAGS_OLD "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})
set(CMAKE_CXX_CLANG_TIDY_OLD ${CMAKE_CXX_CLANG_TIDY})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

  set(CMAKE_CXX_CLANG_TIDY "")
  set(GC_BUILD_SHARED_LIBS OFF)

  # Malloc redirection is causing crashes on macOS. However, without it, we get
  # premature collections. Because of this, collection is disabled on macOS entirely.
  #
  # https://github.com/bdwgc/bdwgc/issues/829
  if(NOT APPLE)
    set(enable_redirect_malloc ON CACHE BOOL "Redirect malloc and friends to collector routines")
    set(enable_uncollectable_redirection ON CACHE BOOL "Redirect to uncollectible malloc instead of garbage-collected one")
  endif()

  set(enable_cplusplus ON CACHE BOOL "Enable C++")
  set(build_cord OFF CACHE BOOL "Build cord")
  set(enable_docs OFF CACHE BOOL "Enable docs")
  set(enable_threads ON CACHE BOOL "Enable multi-threading support")
  set(enable_large_config ON CACHE BOOL "Optimize for large heap or root set")
  set(enable_throw_bad_alloc_library ON CACHE BOOL "Enable C++ gctba library build")
  set(enable_gc_debug OFF CACHE BOOL "Support for pointer back-tracing")

  if(jank_profile_gc)
    set(enable_valgrind_tracking ON CACHE BOOL "Support tracking GC_malloc and friends for heap profiling tools")
  endif()

  if(jank_debug_gc)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGC_ASSERTIONS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGC_ASSERTIONS")
    set(enable_gc_debug ON CACHE BOOL "Support for pointer back-tracing")
  endif()

  add_subdirectory(third-party/bdwgc EXCLUDE_FROM_ALL)

  unset(GC_BUILD_SHARED_LIBS)
  unset(enable_redirect_malloc)
  unset(enable_uncollectable_redirection)
  unset(enable_cplusplus)
  unset(build_cord)
  unset(enable_docs)
  unset(enable_threads)
  unset(enable_large_config)
  unset(enable_throw_bad_alloc_library)
  unset(enable_valgrind_tracking)
  unset(enable_gc_debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_OLD}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_OLD}")
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_OLD})
set(CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY_OLD})
