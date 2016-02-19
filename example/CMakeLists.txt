option(SPICA_BUILD_EXAMPLE "BUILD_EXAMPLE" ON)

if (${SPICA_BUILD_EXAMPLE} STREQUAL "ON")
  # Link directory
  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  function (generate_example example_name)
    add_executable(${example_name} "${example_name}_example.cc")
    add_dependencies(${example_name} ${SPICA_TARGET_NAME})
    target_link_libraries(${example_name} "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")

    if (MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
      set_property(TARGET ${example_name} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
      set_property(TARGET ${example_name} PROPERTY FOLDER "Examples")
    endif()
  endfunction()

  generate_example(pathtracing)
  # generate_example(bdpt)
  # generate_example(mlt)
  # generate_example(photonmap)
  generate_example(sppm)

  include_directories(${CMAKE_CURRENT_LIST_DIR})
endif()
