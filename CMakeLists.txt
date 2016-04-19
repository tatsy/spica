cmake_minimum_required(VERSION 3.0.0)
project(spica)

# ------------------------------------------------------------------------------
# Set compiler options
# ------------------------------------------------------------------------------
if (NOT WIN32)
  # Linux or Mac OS X
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp=libomp")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
  endif()
endif()

# ------------------------------------------------------------------------------
# Whether use static libraries or not
# ------------------------------------------------------------------------------
set(spica_USE_STATIC_LIBS OFF CACHE BOOL "")

# ------------------------------------------------------------------------------
# Set library prefices and suffices
# ------------------------------------------------------------------------------
if (WIN32)
  set(LIB_PREFIX "")
  set(LIB_SUFFIX ".lib")
else()
  if (spica_USE_STATIC_LIBS)
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
# Set output path
# ------------------------------------------------------------------------------
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

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
include_directories(${STB_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
# Import Boost libraries
# ------------------------------------------------------------------------------
if (MSVC)
  set(BOOST_ROOT ${BOOST_ROOT} CACHE PATH "BOOST-NOT_FOUND")
  if (NOT EXISTS "${BOOST_ROOT}/boost")
	message(FATAL_ERROR "[ERROR] Boost not found!!")
  else()
	message(STATUS "Boost: ${BOOST_ROOT}")
	set(Boost_USE_STATIC_LIBS FALSE CACHE BOOL FALSE)
  endif()
endif()
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# ------------------------------------------------------------------------------
# Include subdirectories
# ------------------------------------------------------------------------------
add_subdirectory(sources)
add_subdirectory(test)
add_subdirectory(example)
