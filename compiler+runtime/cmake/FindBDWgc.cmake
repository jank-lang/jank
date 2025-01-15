# Try to find the bdwgc libraries
# See https://github.com/ivmai/bdwgc
#
# This file sets up gc for CMake. Once done this will define
#  BDWGC_FOUND             - system has BDWGC lib
#  BDWGC_INCLUDE_DIR       - the BDWGC include directory
#  BDWGC_LIBRARIES         - Libraries needed to use BDWGC
#
# Copyright (c) 2020, Mahrud Sayrafi, <mahrud@umn.edu>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# Downloaded from here: https://sources.debian.org/data/main/m/macaulay2/1.17.1%2Bds-2/M2/cmake/FindBDWGC.cmake

set(BDWGC_INCLUDE_DIR NOTFOUND)
set(BDWGC_LIBRARIES NOTFOUND)

# search first if an BDWgcConfig.cmake is available in the system,
# if successful this would set BDWGC_INCLUDE_DIR and the rest of
# the script will work as usual
find_package(BDWgc NO_MODULE QUIET)

if(NOT BDWGC_INCLUDE_DIR)
  find_path(BDWGC_INCLUDE_DIR NAMES gc/gc.h
    HINTS ENV BDWGCDIR
    PATHS ${INCLUDE_INSTALL_DIR} ${CMAKE_INSTALL_PREFIX}/include
    PATH_SUFFIXES gc bdwgc
    )
endif()

if(NOT BDWGC_LIBRARIES)
  find_library(BDWGC_GC_LIBRARY NAMES gc
    REQIURED
    HINTS ENV BDWGCDIR
    PATHS ${LIB_INSTALL_DIR} ${CMAKE_INSTALL_PREFIX}/lib ${BDWGC_INCLUDE_DIR}/../lib
    )
  find_library(BDWGC_GCCPP_LIBRARY NAMES gccpp
    REQIURED
    HINTS ENV BDWGCDIR
    PATHS ${LIB_INSTALL_DIR} ${CMAKE_INSTALL_PREFIX}/lib ${BDWGC_INCLUDE_DIR}/../lib
    )
  set(BDWGC_LIBRARIES ${BDWGC_GC_LIBRARY} ${BDWGC_GCCPP_LIBRARY})
endif()

# Query BDWGC_VERSION
file(READ "${BDWGC_INCLUDE_DIR}/gc/gc_version.h" _gc_version_header)
string(REGEX MATCH "define[ \t]+GC_TMP_VERSION_MAJOR[ \t]+([0-9]+)"
       _gc_major_version_match "${_gc_version_header}")
set(BDWGC_MAJOR_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+GC_TMP_VERSION_MINOR[ \t]+([0-9]+)"
       _gc_minor_version_match "${_gc_version_header}")
set(BDWGC_MINOR_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+GC_TMP_VERSION_MICRO[ \t]+([0-9]+)"
       _gc_micro_version_match "${_gc_version_header}")
set(BDWGC_MICRO_VERSION "${CMAKE_MATCH_1}")
set(BDWGC_VERSION ${BDWGC_MAJOR_VERSION}.${BDWGC_MINOR_VERSION}.${BDWGC_MICRO_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BDWgc
  REQUIRED_VARS BDWGC_INCLUDE_DIR BDWGC_LIBRARIES
  VERSION_VAR BDWGC_VERSION)

mark_as_advanced(BDWGC_INCLUDE_DIR BDWGC_LIBRARIES)
