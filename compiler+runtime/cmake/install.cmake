# When the compiler is installed, it needs to be relinked to its shared objects.
# We know where they'll be, relative to the compiler, though.
set_target_properties(jank_exe PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

install(
  TARGETS jank_exe
  # Tempting to do this, to include all dynamic libs, but it doesn't reliably work.
  #RUNTIME_DEPENDENCIES DIRECTORIES
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/jank
  DESTINATION src
)

# So vcpkg puts all of the headers we need for all of our dependencies
# into one include prefix based on our target triplet. This is excellent,
# since we need to package all of those headers along with jank. Why?
# Because JIT compilation means that any headers we need at compile-time
# we'll also need at run-time.

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
    install(FILES ${header} DESTINATION "${jank_glob_OUTPUT_PREFIX}${relative_dir}")
  endforeach(header)
endfunction()

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/"
  PATTERN "${CMAKE_SOURCE_DIR}/include/cpp/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/"
  PATTERN "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/nanobench/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/nanobench/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/bpptree/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/bpptree/include/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/folly/"
  OUTPUT_PREFIX "include/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/folly/folly/*.h"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CLANG_INSTALL_PREFIX}/"
  PATTERN "${CLANG_INSTALL_PREFIX}/include/*"
)

install(FILES ${CMAKE_SOURCE_DIR}/clang-format DESTINATION share)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
