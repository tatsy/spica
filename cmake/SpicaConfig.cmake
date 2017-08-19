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
include(CMakeParseArguments)

set(SPICA_PREFIX "spica")

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

# ------------------------------------------------------------------------------
# Message macro
# ------------------------------------------------------------------------------
macro(spica_status_message)
  message(STATUS ${ARGN})
endmacro()

macro(spica_warning_message)
  message(WARNING ${ARGN})
endmacro()

macro(spica_error_message)
  message(FATAL_ERROR ${ARGN})
endmacro()

# ------------------------------------------------------------------------------
# spica's install destination
# ------------------------------------------------------------------------------
set(SPICA_PLUGIN_DEST "${CMAKE_BINARY_DIR}/bin/plugins")
set(SPICA_CORELIB_DEST "${CMAKE_BINARY_DIR}/sdk")

# ------------------------------------------------------------------------------
# spica's core library building
# ------------------------------------------------------------------------------
macro(add_spica_corelib _corelib_name)
  CMAKE_PARSE_ARGUMENTS(_corelib "TYPE" "LINK_LIBRARIES" ${ARGV})
  set(_corelib_srcs ${_corelib_UNPARSED_ARGUMENTS})

  add_library(${_corelib_name} SHARED ${_corelib_srcs})
  source_group("Source Files" FILES ${_corelib_srcs})

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET ${_corelib_name} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
  endif()

  if (_corelib_LINK_LIBRARIES)
    target_link_libraries(${_corelib_name} ${_corelib_LINK_LIBRARIES})
    add_dependencies(${_corelib_name} ${_corelib_LINK_LIBRARIES})
  endif()

  # Installation
  install(TARGETS ${_corelib_name}
          RUNTIME DESTINATION ${SPICA_CORELIB_DEST} COMPONENT Runtime
          LIBRARY DESTINATION ${SPICA_CORELIB_DEST} COMPONENT Runtime)
endmacro()

# ------------------------------------------------------------------------------
# spica's plugin building
# ------------------------------------------------------------------------------
macro(add_spica_plugin _plugin_name)
  CMAKE_PARSE_ARGUMENTS(_plugin "" "TYPE" "LINK_LIBRARIES" ${ARGN})
  set(_plugin_srcs ${_plugin_UNPARSED_ARGUMENTS})

  # Define library
  spica_status_message("Plugin: ${_plugin_TYPE}/${_plugin_name}")
  add_library(${_plugin_name} MODULE ${_plugin_srcs})
  source_group("Source Files" FILES ${_plugin_srcs})
  set_target_properties(${_plugin_name} PROPERTIES PREFIX "")

  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET ${_plugin_name} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
  endif()

  # Link setting
  set(_plugin_core_libraries "${SPICA_PREFIX}_core")
  target_link_libraries(${_plugin_name} ${_plugin_core_libraries})

  # Folder setting
  set(_plugin_FOLDER "plugins")
  if (_plugin_TYPE)
    if (CMAKE_GENERATOR MATCHES "Visual Studio")
      set(_plugin_FOLDER "plugins/${_plugin_TYPE}")
    else()
      set_target_properties(${_plugin_name} PROPERTIES PROJECT_LABEL "${_plugin_TYPE}-${_plugin_name}")
    endif()
  endif()
  set_target_properties(${_plugin_name} PROPERTIES FOLDER ${_plugin_FOLDER})
  unset(_plugin_FOLDER)

  # Installation
  install(TARGETS ${_plugin_name}
          RUNTIME DESTINATION ${SPICA_PLUGIN_DEST} COMPONENT Runtime
          LIBRARY DESTINATION ${SPICA_PLUGIN_DEST} COMPONENT Runtime)
endmacro()
