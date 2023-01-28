option(jank_coverage "Enable code coverage measurement" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "coverage" OR jank_coverage)
  list(APPEND jank_compiler_flags -fprofile-instr-generate -fcoverage-mapping)
endif()
