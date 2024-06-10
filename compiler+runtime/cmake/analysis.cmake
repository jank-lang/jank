option(jank_analysis "Enable static analysis" OFF)
if(jank_analysis)
  set(CMAKE_CXX_CLANG_TIDY $ENV{CC}-tidy --use-color)
endif()
