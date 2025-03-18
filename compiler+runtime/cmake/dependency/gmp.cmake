# GMP version to use
set(GMP_VERSION "6.3.0")
set(GMP_URL "https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.xz")

# Set up GMP build directory
set(GMP_PREFIX "${CMAKE_BINARY_DIR}/third-party/gmp")
set(GMP_SOURCE_DIR "${GMP_PREFIX}/src/gmp_external")
set(GMP_BINARY_DIR "${GMP_PREFIX}/src/gmp_external-build")
set(GMP_INSTALL_DIR "${GMP_PREFIX}")
set(GMP_INCLUDE_DIR "${GMP_INSTALL_DIR}/include")
set(GMP_LIB_DIR "${GMP_INSTALL_DIR}/lib")

# Create the GMP static library targets
add_library(gmp_static STATIC IMPORTED GLOBAL)
add_library(gmpxx_static STATIC IMPORTED GLOBAL)

# Set the imported library locations
set_target_properties(gmp_static PROPERTIES
    IMPORTED_LOCATION "${GMP_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmp${CMAKE_STATIC_LIBRARY_SUFFIX}"
    INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
)

set_target_properties(gmpxx_static PROPERTIES
    IMPORTED_LOCATION "${GMP_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmpxx${CMAKE_STATIC_LIBRARY_SUFFIX}"
    INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES gmp_static
)

# Download and build GMP
include(ExternalProject)
ExternalProject_Add(
    gmp_external
    URL ${GMP_URL}
    PREFIX ${GMP_PREFIX}
    SOURCE_DIR ${GMP_SOURCE_DIR}
    BINARY_DIR ${GMP_BINARY_DIR}
    CONFIGURE_COMMAND ${GMP_SOURCE_DIR}/configure
                      --prefix=${GMP_INSTALL_DIR}
                      --enable-static
                      --disable-shared
                      --with-pic
                      --enable-cxx
    BUILD_COMMAND make -j${CMAKE_BUILD_PARALLEL_LEVEL}
    INSTALL_COMMAND make install
    BUILD_BYPRODUCTS
        "${GMP_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmp${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${GMP_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmpxx${CMAKE_STATIC_LIBRARY_SUFFIX}"
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Add dependencies
add_dependencies(gmp_static gmp_external)
add_dependencies(gmpxx_static gmp_external)

# Make the include directory available
include_directories(SYSTEM ${GMP_INCLUDE_DIR})
