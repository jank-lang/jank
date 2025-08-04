# We don't have any dynamic lib deps which are installed alongside jank.
set(CMAKE_SKIP_INSTALL_RPATH ON)
install(TARGETS jank_exe DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/libjank.a DESTINATION lib/jank/${PROJECT_VERSION}/lib)

# This is a helper which recursively takes headers from one directory
# and installs them into the same output include directory. It
# handles various different situations of include/ or not, supports
# globbing, etc.
function(jank_glob_install_without_prefix)
  set(options )
  set(one_value_args INPUT_PREFIX OUTPUT_PREFIX PATTERN)
  set(multi_value_args )
  cmake_parse_arguments(jank_glob "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  file(GLOB_RECURSE jank_glob_result ${jank_glob_PATTERN})
  string(LENGTH "${jank_glob_INPUT_PREFIX}" prefix_length)
  foreach(header ${jank_glob_result})
    string(SUBSTRING ${header} ${prefix_length} -1 header_without_prefix)
    cmake_path(GET header_without_prefix PARENT_PATH relative_dir)
    install(FILES ${header} DESTINATION "lib/jank/${PROJECT_VERSION}/${jank_glob_OUTPUT_PREFIX}${relative_dir}")
  endforeach(header)
endfunction()

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/include/cpp/"
  OUTPUT_PREFIX "include/"
  PATTERN "${CMAKE_SOURCE_DIR}/include/cpp/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/"
  PATTERN "${CMAKE_SOURCE_DIR}/src/jank/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/bdwgc/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/bdwgc/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/nanobench/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/nanobench/include/*"
)

# TODO: Trim this WAY down to only what we use.
jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/folly/"
  OUTPUT_PREFIX "include/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/folly/folly/*.h"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/bpptree/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/bpptree/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/immer/"
  OUTPUT_PREFIX "include/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/immer/immer/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/cli11/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/cli11/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/ftxui/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/ftxui/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/libzippp/src/"
  OUTPUT_PREFIX "include/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/libzippp/src/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/cpptrace/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/cpptrace/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/cppinterop/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/cppinterop/include/*"
)
jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/cppinterop/lib/"
  OUTPUT_PREFIX "include/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/cppinterop/lib/*.h"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/boost-preprocessor/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/boost-preprocessor/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/boost-multiprecision/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/boost-multiprecision/include/*"
)

# This is used for formatting C++ code at runtime.
install(FILES ${CMAKE_SOURCE_DIR}/../.clang-format DESTINATION share)

if(jank_local_clang)
  # When the compiler is installed, it needs to be relinked to its shared objects.
  # We know where they'll be, relative to the compiler, though.
  set(CMAKE_SKIP_INSTALL_RPATH OFF)
  set_target_properties(jank_exe PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/jank/${PROJECT_VERSION}/lib")

  install(
    FILES
    ${llvm_dir}/lib/libLLVM.so.${LLVM_VERSION_MAJOR}.0git
    ${llvm_dir}/lib/libclang-cpp.so.${LLVM_VERSION_MAJOR}.0git
    DESTINATION lib/jank/${PROJECT_VERSION}/lib
  )
endif()

# This is included for distro packagers and anyone else packaging jank who wants to use it.
if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
