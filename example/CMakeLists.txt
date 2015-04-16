# Link directory
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(simplept simplept_example.cc)

target_link_libraries(simplept "${LIB_PREFIX}spica_renderer${LIB_SUFFIX}")

include_directories(${CMAKE_CURRENT_LIST_DIR})
