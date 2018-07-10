include_directories(${SPICA_ROOT_DIR}/sources)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

file(GLOB SOURCE_FILES "*.cc" "*.h")

add_executable(${SPICA_MAIN} ${SOURCE_FILES} ${SUBMODULE_SOURCE_FILES})
add_dependencies(${SPICA_MAIN} ${SPICA_LIBCORE} ${SPICA_LIBRENDER})
target_link_libraries(${SPICA_MAIN} ${SPICA_LIBCORE})

source_group("Source Files" FILES ${SOURCE_FILES} ${SUBMODULE_SOURCE_FILES})

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set_property(TARGET ${SPICA_MAIN} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
endif()
