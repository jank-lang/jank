if(CMAKE_BUILD_TYPE STREQUAL "coverage" OR jank_coverage)
  list(APPEND jank_aot_compiler_flags -fprofile-instr-generate -fcoverage-mapping)
  list(APPEND jank_linker_flags -fprofile-instr-generate -fcoverage-mapping)
endif()
