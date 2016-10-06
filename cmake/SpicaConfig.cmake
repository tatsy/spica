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

if(CMAKE_VERSION VERSION_GREATER 2.6)
  get_property(OpenCV_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
  if(NOT ";${OpenCV_LANGUAGES};" MATCHES ";CXX;")
    enable_language(CXX)
  endif()
endif()

if (NOT DEFINED spica_BUILD_STATIC)
  if (BUILD_SHARED_LIBS)
    set(spica_BUILD_STATIC OFF)
  else()
    set(spica_BUILD_STATIC ON)
  endif()
endif()

if (WIN32)
  set(LIB_PREFIX "")
  set(LIB_SUFFIX ".lib")
elseif(MACOS)
  set(LIB_PREFIX "lib")
  set(LIB_SUFFIX ".dylib")
else(LINUX)
  if (spica_BUILD_STATIC)
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".a")
  else()
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX ".so")
  endif()
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
