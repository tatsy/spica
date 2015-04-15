# Link directory
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(simplept simplept_example.cc)

target_link_libraries(simplept "${LIB_PREFIX}rainy_renderer${LIB_SUFFIX}")

add_custom_target(render COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS simplept)

include_directories(${CMAKE_CURRENT_LIST_DIR})
