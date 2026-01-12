# We don't have any dynamic lib deps which are installed alongside jank.
set(CMAKE_SKIP_INSTALL_RPATH ON)
install(TARGETS jank_exe_phase_2 DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/libjank-standalone.a DESTINATION lib/jank/${PROJECT_VERSION}/lib)

function(install_symlink target link_name dir)
  install(CODE "
    set(libdir \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${dir}\")
    execute_process(
      COMMAND \${CMAKE_COMMAND} -E create_symlink
        ${target}
        \"\${libdir}/${link_name}\"
    )
  ")
endfunction()

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
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/immer/"
  OUTPUT_PREFIX "include/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/immer/immer/*"
)

jank_glob_install_without_prefix(
  INPUT_PREFIX "${CMAKE_SOURCE_DIR}/third-party/ftxui/"
  PATTERN "${CMAKE_SOURCE_DIR}/third-party/ftxui/include/*"
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

# If we've built jank with a local Clang/LLVM, we can't reasonably expect the target system
# to have our custom Clang. In this case, the default behavior is to install Clang alongside
# jank, within jank's resource dir. We copy Clang as well as Clang's resource dir and jank
# has custom logic to find Clang in that installed location.
#
# This makes the jank install significantly larger, but it's really our only sane distribution
# approach for local Clang builds. If desired, this can be disabled via jank_install_local_clang.
# For example, if you're installing jank on the current machine, rather than some other machine,
# you could just install jank and its configured Clang will be the Clang used to build jank.
if(jank_local_clang AND jank_install_local_clang)
  # When the compiler is installed, it needs to be relinked to its shared objects.
  # We know where they'll be, relative to the compiler, though.
  set(CMAKE_SKIP_INSTALL_RPATH OFF)

  # macOS doesn't use $ORIGIN in the same way as Linux. CMake doesn't provide a way to generalize.
  # Details are here: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
  if(APPLE)
    set_target_properties(jank_exe_phase_2 PROPERTIES INSTALL_RPATH "@executable_path/../lib/jank/${PROJECT_VERSION}/lib")
  else()
    set_target_properties(jank_exe_phase_2 PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/jank/${PROJECT_VERSION}/lib")
  endif()

  install(
    PROGRAMS
    ${llvm_dir}/bin/clang
    ${llvm_dir}/bin/clang++
    ${llvm_dir}/bin/clang-${LLVM_VERSION_MAJOR}
    DESTINATION lib/jank/${PROJECT_VERSION}/bin
  )

if(APPLE)
  install(
    FILES
    ${llvm_dir}/lib/libLLVM.dylib
    ${llvm_dir}/lib/libclang-cpp.dylib
    ${llvm_dir}/lib/libc++.1.0.dylib
    ${llvm_dir}/lib/libc++.1.dylib
    ${llvm_dir}/lib/libc++.dylib
    ${llvm_dir}/lib/libc++abi.1.0.dylib
    ${llvm_dir}/lib/libc++abi.1.dylib
    ${llvm_dir}/lib/libc++abi.dylib
    ${llvm_dir}/lib/libunwind.1.0.dylib
    ${llvm_dir}/lib/libunwind.1.dylib
    ${llvm_dir}/lib/libunwind.dylib
    DESTINATION lib/jank/${PROJECT_VERSION}/lib
  )
else()
  install(
    FILES
    ${llvm_dir}/lib/libLLVM.so.${LLVM_VERSION_MAJOR}.0git
    ${llvm_dir}/lib/libclang-cpp.so.${LLVM_VERSION_MAJOR}.0git
    DESTINATION lib/jank/${PROJECT_VERSION}/lib
  )

  # Install unversioned symlinks so we can easily just link to -lLLVM.
  install_symlink(libLLVM.so.${LLVM_VERSION_MAJOR}.0git libLLVM.so lib/jank/${PROJECT_VERSION}/lib)
  install_symlink(libclang-cpp.so.${LLVM_VERSION_MAJOR}.0git libclang-cpp.so lib/jank/${PROJECT_VERSION}/lib)
endif()

  jank_glob_install_without_prefix(
    INPUT_PREFIX "${llvm_dir}/"
    PATTERN "${llvm_dir}/include/*"
  )
  jank_glob_install_without_prefix(
    INPUT_PREFIX "${clang_resource_dir}/"
    OUTPUT_PREFIX "lib/clang/${LLVM_VERSION_MAJOR}/"
    PATTERN "${clang_resource_dir}/*"
  )
endif()

# This is included for distro packagers and anyone else packaging jank who wants to use it.
if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
