# =============================================================================
# The spica CMake configuration file
#
#    This file will define the following variables:
#      - spica_LIBS
#      - spica_INCLUDE_DIRS
#
# =============================================================================
#
#    Windows pack specific options:
#      - spica_BUILD_STATIC
#

# ------------------------------------------------------------------------------
# Flag to determine Linux or not
# ------------------------------------------------------------------------------
set(LINUX FALSE)
if (NOT WIN32 AND NOT APPLE)
  set(LINUX TRUE)
endif()

# ------------------------------------------------------------------------------
# Library prefix and suffix
# ------------------------------------------------------------------------------
if (WIN32)
  set(LIB_PREFIX "")
  set(LIB_SUFFIX ".lib")
endif()

if(APPLE)
  set(LIB_PREFIX "lib")
  set(LIB_SUFFIX ".dylib")
endif()

if (LINUX)
  set(LIB_PREFIX "lib")
  set(LIB_SUFFIX ".so")
endif()

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/lib")
  set(spica_FOUND TRUE CACHE BOOL "" FORCE)
  set(SPICA_FOUND TRUE CACHE BOOL "" FORCE)
  set(spica_LIBS
      "${CMAKE_CURRENT_LIST_DIR}/lib/${LIB_PREFIX}spica_renderer${LIB_SUFFIX}"
      CACHE PATH "The spica libraries")
  set(spica_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include"
      CACHE PATH "The spica include paths")
else()
  set(spica_FOUND FALSE CACHE BOOL "" FORCE)
  set(SPICA_FOUND FALSE CACHE BOOL "" FORCE)
endif()
