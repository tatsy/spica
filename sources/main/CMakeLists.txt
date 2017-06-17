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
# Build main
# ------------------------------------------------------------------------------
set(BUILD_TARGET spica_main)

include_directories(${SPICA_ROOT_DIR}/sources)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(${BUILD_TARGET} main.cc)
add_dependencies(${BUILD_TARGET} ${SPICA_TARGET_NAME})
target_link_libraries(${BUILD_TARGET} "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
target_link_libraries(${BUILD_TARGET} ${Boost_LIBRARIES})

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
endif()
