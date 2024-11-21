set(CMAKE_CXX_CLANG_TIDY_OLD ${CMAKE_CXX_CLANG_TIDY})
  set(CMAKE_CXX_CLANG_TIDY "")
  set(FMT_DOC OFF CACHE BOOL "Generate docs")
  set(FMT_INSTALL OFF CACHE BOOL "Generate the install target.")

  add_subdirectory(third-party/fmt)

  unset(FMT_DOC)
  unset(FMT_INSTALL)
set(CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY_OLD})
