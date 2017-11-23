cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(spica)

include(cmake/SpicaConfig.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------
option(SPICA_BUILD_MAIN "Build spica executable." OFF)
option(SPICA_BUILD_TESTS "Build unit tests." OFF)
option(SPICA_BUILD_GUI "Build GUI." OFF)
option(SPICA_WITH_OPENMP "Build with OpenMP" OFF)
option(WITH_SSE "Build with SSE (used in QBVH)" OFF)
option(WITH_FFTW "Build with FFTW (used in GDPT)" OFF)

# ------------------------------------------------------------------------------
# Common build targets
# ------------------------------------------------------------------------------
set(SPICA_MAIN "spica")
set(SPICA_GUI "spica_gui")
set(SPICA_LIBCORE "spica_core")

# ------------------------------------------------------------------------------
# Set compiler options
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
if (NOT WIN32)
  # Linux or Mac OS X
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")
  if (${LINUX} AND ${SPICA_BUILD_TESTS})
    message(STATUS "[spica] Generate coverage data.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
  endif()
endif()

if (WITH_SSE)
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
  endif()
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(SPICA_DEPENDENCY_INCLUDE_DIRS "")
set(SPICA_DEPENDENCY_LIBRARIES "")

if (${SPICA_BUILD_MAIN} OR ${SPICA_BUILD_GUI})
  set(QT5_ROOT "QT5_ROOT" CACHE PATH "QT5_ROOT")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_ROOT})
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
  find_package(Qt5Xml REQUIRED)
  find_package(OpenGL REQUIRED)

  set(SPICA_DEPENDENCY_LIBRARIES ${SPICA_DEPENDENCY_INCLUDE_DIRS} ${QT_LIBRARIES} ${OPENGL_LIBRARIES})
endif()

if (WITH_FFTW)
  find_package(FFTW REQUIRED)
  add_definitions(-DSPICA_WITH_FFTW)
  set(SPICA_DEPENDENCY_INCLUDE_DIRS ${SPICA_DEPENDENCY_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})
  set(SPICA_DEPENDENCY_LIBRARIES ${SPICA_DEPENDENCY_LIBRARIES} ${FFTW_LIBRARIES})
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
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "10.13" FORCE)
endif()

# ------------------------------------------------------------------------------
# Set output path
# ------------------------------------------------------------------------------
#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)

# ------------------------------------------------------------------------------
# Set spica's root directory
# ------------------------------------------------------------------------------
set(SPICA_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# ------------------------------------------------------------------------------
# Set spica's include/library directories
# ------------------------------------------------------------------------------
include_directories("sources")
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# ------------------------------------------------------------------------------
# Import 3rd party libraries
# ------------------------------------------------------------------------------
add_subdirectory(3rdparty)
include_directories(${THIRD_PARTY_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
# Include subdirectories
# ------------------------------------------------------------------------------
add_subdirectory(sources)
add_subdirectory(tests)
