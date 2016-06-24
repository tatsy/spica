set(BUILD_TARGET spica_main)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(${BUILD_TARGET} main.cc)
add_dependencies(${BUILD_TARGET} ${SPICA_TARGET_NAME})
target_link_libraries(${BUILD_TARGET} "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
target_link_libraries(${BUILD_TARGET} ${Boost_LIBRARIES})
message(STATUS ${Boost_LIBRARIES})

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
endif()
