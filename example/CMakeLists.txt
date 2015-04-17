# Link directory
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

function (generate_example example_name)
  add_executable(${example_name} "${example_name}_example.cc")
  target_link_libraries(${example_name} "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
  configure_file(project.vcxproj.user.in "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${example_name}.vcxproj.user" @ONLY)

  endif()
endfunction()

add_executable(simplept simplept_example.cc)
add_executable(simplebpt simplebpt_example.cc)

target_link_libraries(simplept "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")
target_link_libraries(simplebpt "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")

include_directories(${CMAKE_CURRENT_LIST_DIR})
