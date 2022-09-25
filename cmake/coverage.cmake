if(CMAKE_BUILD_TYPE STREQUAL "coverage" OR jank_coverage)
  if("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    message("Building with llvm Code Coverage Tools")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")

  elseif(CMAKE_COMPILER_IS_GNUCXX)
    message("Building with lcov Code Coverage Tools")

    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
  else()
    message(FATAL_ERROR "Code coverage requires Clang or GCC. Aborting.")
  endif()
endif()
