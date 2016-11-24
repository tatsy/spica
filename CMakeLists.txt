cmake_minimum_required(VERSION 3.0.0)
project(spica)

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------
option(SPICA_BUILD_TESTS "Build unit tests." OFF)
option(SPICA_BUILD_GUI "Build GUI." OFF)

# ------------------------------------------------------------------------------
# Set compiler options
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
if (NOT WIN32)
  # Linux or Mac OS X
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")
  if (${SPICA_BUILD_TESTS})
    message(STATUS "[spica] Generate coverage data.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
  endif()
endif()

# ------------------------------------------------------------------------------
# Find OpenMP
# ------------------------------------------------------------------------------
set(SPICA_USE_OPENMP OFF CACHE BOOL OFF)
if (SPICA_USE_OPENMP)
  find_package(OpenMP)
  if (OPENMP_FOUND)
    message(STATUS "OpenMP Supported!!")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

# ------------------------------------------------------------------------------
# Whether use static libraries or not
# ------------------------------------------------------------------------------
set(SPICA_USE_STATIC_LIBS OFF CACHE BOOL OFF)

# ------------------------------------------------------------------------------
# Set library prefices and suffices
# ------------------------------------------------------------------------------
if (WIN32)
  set(LIB_PREFIX "")
  set(LIB_SUFFIX ".lib")
elseif(APPLE)
  if (SPICA_USE_STATIC_LIBS)
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".a")
  else()
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".dylib")
  endif()
else()
  if (SPICA_USE_STATIC_LIBS)
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".a")
  else()
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".so")
  endif()
endif()

# ------------------------------------------------------------------------------
# Special settings for MSVC
# ------------------------------------------------------------------------------
if (MSVC)
  # Use solution folders
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  # Disable comparatively unimportant warning
  add_definitions("/D_CRT_SECURE_NO_WARNINGS /W3 /wd4251")
endif()

# ------------------------------------------------------------------------------
# Special settings for MacOSX
# ------------------------------------------------------------------------------
if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "10.11" FORCE)
endif()

# ------------------------------------------------------------------------------
# Set output path
# ------------------------------------------------------------------------------
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)

# ------------------------------------------------------------------------------
# Set spica's root directory
# ------------------------------------------------------------------------------
set(SPICA_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# ------------------------------------------------------------------------------
# Create output directory
# ------------------------------------------------------------------------------
file(MAKE_DIRECTORY output)

# ------------------------------------------------------------------------------
# Import 3rd party libraries
# ------------------------------------------------------------------------------
add_subdirectory(3rdparty)
include_directories(${THIRD_PARTY_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
# Import Boost libraries
# ------------------------------------------------------------------------------
if (MSVC)
  set(BOOST_ROOT ${BOOST_ROOT} CACHE PATH "BOOST-NOT_FOUND")
  if (NOT EXISTS "${BOOST_ROOT}/boost")
	  message(FATAL_ERROR "[ERROR] Boost not found!!")
  else()
    message(STATUS "Boost: ${BOOST_ROOT}")
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
    set(Boost_USE_STATIC_LIBS FALSE CACHE BOOL FALSE)
  endif()
endif()
find_package(Boost REQUIRED system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
# Include subdirectories
# ------------------------------------------------------------------------------
add_subdirectory(sources)
add_subdirectory(tests)
