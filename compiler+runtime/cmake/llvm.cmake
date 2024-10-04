# LLVM/Clang default paths
if (NOT DEFINED llvm_dir)
  set(llvm_dir "${CMAKE_BINARY_DIR}/llvm-install/usr/local")
endif()
if (NOT DEFINED clang_dir)
  set(clang_dir ${llvm_dir})
endif()

## Define supported version of clang and llvm
set(CLANG_MIN_SUPPORTED 19.0)
set(CLANG_MAX_SUPPORTED "19.1.1")
set(CLANG_VERSION_UPPER_BOUND 19.1.1)
set(LLVM_MIN_SUPPORTED 19.0)
set(LLVM_MAX_SUPPORTED "19.1.1")
set(LLVM_VERSION_UPPER_BOUND 19.1.1)

#set(LLVM_ENABLE_EH YES)
set(LLVM_REQUIRES_RTTI YES)

## Set Cmake packages search order

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

## Search packages HINTS and PATHS

if (DEFINED llvm_dir)
  set(llvm_search_hints PATHS ${llvm_dir} HINTS "${llvm_dir}/lib/cmake/llvm" "${llvm_dir}/cmake" "${LLVM_CONFIG_EXTRA_PATH_HINTS}")
  set(clang_search_hints PATHS ${llvm_dir} HINTS "${llvm_dir}/lib/cmake/llvm" "${llvm_dir}/cmake" "${LLVM_CONFIG_EXTRA_PATH_HINTS}")
endif()
if (DEFINED clang_dir)
  set(clang_search_hints PATHS ${clang_dir} HINTS "${clang_dir}/lib/cmake/clang" "${clang_dir}/cmake" "${CLANG_CONFIG_EXTRA_PATH_HINTS}")
endif()

## Find supported LLVM

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if (LLVM_FOUND)
  if (LLVM_PACKAGE_VERSION VERSION_LESS LLVM_MIN_SUPPORTED OR LLVM_PACKAGE_VERSION VERSION_GREATER_EQUAL LLVM_VERSION_UPPER_BOUND)
    unset(LLVM_FOUND)
    unset(LLVM_VERSION_MAJOR)
    unset(LLVM_VERSION_MINOR)
    unset(LLVM_VERSION_PATCH)
    unset(LLVM_PACKAGE_VERSION)
  else()
    if (NOT DEFINED LLVM_VERSION AND NOT DEFINED llvm_dir)
      set(LLVM_VERSION ${LLVM_PACKAGE_VERSION})
    endif()
  endif()
endif()

if (NOT LLVM_FOUND AND DEFINED LLVM_VERSION)
  if (LLVM_VERSION VERSION_GREATER_EQUAL LLVM_VERSION_UPPER_BOUND)
    set(LLVM_VERSION ${LLVM_VERSION_UPPER_BOUND})
  endif()
  if (LLVM_VERSION VERSION_LESS LLVM_MIN_SUPPORTED)
    set(LLVM_VERSION ${LLVM_MIN_SUPPORTED})
  endif()

  find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG ${llvm_search_hints} NO_DEFAULT_PATHS)
endif()

if (NOT LLVM_FOUND AND DEFINED llvm_dir)
  find_package(LLVM REQUIRED CONFIG ${llvm_search_hints} NO_DEFAULT_PATH)
endif()

if (NOT LLVM_FOUND)
  find_package(LLVM REQUIRED CONFIG)
endif()

if (NOT LLVM_FOUND)
  message(FATAL_ERROR "Please set llvm_dir pointing to the LLVM build or installation folder")
endif()

if (LLVM_PACKAGE_VERSION VERSION_LESS LLVM_MIN_SUPPORTED OR LLVM_PACKAGE_VERSION VERSION_GREATER LLVM_VERSION_UPPER_BOUND)
  message(FATAL_ERROR "Found unsupported version: LLVM ${LLVM_PACKAGE_VERSION};\nPlease set llvm_dir pointing to the llvm version ${LLVM_MIN_SUPPORTED} to ${LLVM_MAX_SUPPORTED} build or installation folder")
endif()

message(STATUS "Found supported version: LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${llvm_dir}")
message(STATUS "Clang install prefix: ${CLANG_INSTALL_PREFIX}")

## Find supported Clang

if (DEFINED CLANG_VERSION)
  if (CLANG_VERSION VERSION_GREATER_EQUAL CLANG_VERSION_UPPER_BOUND)
    set(CLANG_VERSION ${CLANG_VERSION_UPPER_BOUND})
  endif()
  if (CLANG_VERSION VERSION_LESS CLANG_MIN_SUPPORTED)
    set(CLANG_VERSION ${CLANG_MIN_SUPPORTED})
  endif()

  find_package(Clang ${CLANG_VERSION} REQUIRED CONFIG ${clang_extra_hints} NO_DEFAULT_PATH)
endif()

if (NOT Clang_FOUND AND DEFINED clang_dir)
  find_package(Clang REQUIRED CONFIG ${clang_extra_hints} NO_DEFAULT_PATH)
endif()

if (NOT Clang_FOUND)
  find_package(Clang REQUIRED CONFIG)
endif()

if (NOT Clang_FOUND)
  message(FATAL_ERROR "Please set clang_dir pointing to the clang build or installation folder")
endif()

set(CLANG_VERSION_MAJOR ${LLVM_VERSION_MAJOR})
set(CLANG_VERSION_MINOR ${LLVM_VERSION_MINOR})
set(CLANG_VERSION_PATCH ${LLVM_VERSION_PATCH})
set(CLANG_PACKAGE_VERSION ${LLVM_PACKAGE_VERSION})

if (CLANG_PACKAGE_VERSION VERSION_LESS CLANG_MIN_SUPPORTED OR CLANG_PACKAGE_VERSION VERSION_GREATER CLANG_VERSION_UPPER_BOUND)
  message(FATAL_ERROR "Found unsupported version: Clang ${CLANG_PACKAGE_VERSION};\nPlease set clang_dir pointing to the clang version ${CLANG_MIN_SUPPORTED} to ${CLANG_MAX_SUPPORTED} build or installation folder")
endif()

message(STATUS "Found supported version: Clang ${CLANG_PACKAGE_VERSION}")
message(STATUS "Using ClangConfig.cmake in: ${clang_dir}")

## Clang 13 require c++14 or later, Clang 16 require c++17 or later.
if (CLANG_VERSION_MAJOR GREATER_EQUAL 16)
  if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 17)
  endif()
  if (CMAKE_CXX_STANDARD LESS 17)
    message(fatal "LLVM/InterOp requires c++17 or later")
  endif()
elseif (CLANG_VERSION_MAJOR GREATER_EQUAL 13)
  if (NOT CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 14)
  endif()
  if (CMAKE_CXX_STANDARD LESS 14)
    message(fatal "LLVM/InterOp requires c++14 or later")
  endif()
endif()

## Init

# In case this was a path to a build folder of llvm still try to find AddLLVM
list(APPEND CMAKE_MODULE_PATH "${llvm_dir}")

# Fix bug in some AddLLVM.cmake implementation (-rpath "" problem)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})

include(AddLLVM)
include(HandleLLVMOptions)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(SYSTEM ${CLANG_INCLUDE_DIRS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

message(STATUS "CLANG_INCLUDE_DIRS: ${CLANG_INCLUDE_DIRS}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_DEFINITIONS_LIST: ${LLVM_DEFINITIONS_LIST}")

# If the llvm sources are present add them with higher priority.
if (LLVM_BUILD_MAIN_SRC_DIR)
  # LLVM_INCLUDE_DIRS contains the include paths to both LLVM's source and
  # build directories. Since we cannot just include ClangConfig.cmake (see
  # fixme above) we have to do a little more work to get the right include
  # paths for clang.
  #
  # FIXME: We only support in-tree builds of clang, that is clang being built
  # in llvm_src/tools/clang.
  include_directories(SYSTEM ${LLVM_BUILD_MAIN_SRC_DIR}/tools/clang/include/)

  if (NOT LLVM_BUILD_BINARY_DIR)
    message(FATAL "LLVM_BUILD_* values should be available for the build tree")
  endif()

  include_directories(SYSTEM ${LLVM_BUILD_BINARY_DIR}/tools/clang/include/)
endif()

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (APPLE)
  set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-flat_namespace -Wl,-undefined -Wl,suppress")
endif ()
