# Link directory
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

function (generate_example example_name)
  add_executable(${example_name} "${example_name}_example.cc")
  target_link_libraries(${example_name} "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
  configure_file(${CMAKE_SOURCE_DIR}/template/project.vcxproj.user.in "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${example_name}.vcxproj.user" @ONLY)
endfunction()

generate_example(simplept)
generate_example(simplebpt)
generate_example(simplemlt)

include_directories(${CMAKE_CURRENT_LIST_DIR})
