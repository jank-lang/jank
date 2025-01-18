option(jank_analysis "Enable static analysis" OFF)
if(jank_analysis)
  if(NOT DEFINED CMAKE_CXX_CLANG_TIDY)
    # In case clang-tidy exists wherever clang is, just try there.
    # Otherwise, fall back to the default of searching in PATH.
    if(EXISTS $ENV{CC}-tidy)
      set(CMAKE_CXX_CLANG_TIDY $ENV{CC}-tidy --use-color)
    endif()
  endif()
endif()
