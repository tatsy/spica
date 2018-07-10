# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
include_directories(${SPICA_DEPENDENCY_INCLUDE_DIRS})

set(SUBMODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/submodules")
file(GLOB CHILDREN "${SUBMODULE_DIR}/*")
foreach(F ${CHILDREN})
    if(IS_DIRECTORY ${F})
        include_directories(${F})
    endif()
endforeach()

file(GLOB SUBMODULE_SOURCE_FILES "${SUBMODULE_DIR}/**/*.cpp" "${SUBMODULE_DIR}/**/*.h")
list(FILTER SUBMODULE_SOURCE_FILES EXCLUDE REGEX ".*xmltest.cpp$")
message(STATUS ${SUBMODULE_SOURCE_FILES})

# ------------------------------------------------------------------------------
# Process subdirectories
# ------------------------------------------------------------------------------
add_subdirectory(core)

# ------------------------------------------------------------------------------
# Plugins
# ------------------------------------------------------------------------------
add_subdirectory(cameras)
add_subdirectory(lights)
add_subdirectory(accelerators)
add_subdirectory(integrators)
add_subdirectory(shapes)
add_subdirectory(samplers)
add_subdirectory(films)
add_subdirectory(filters)
add_subdirectory(bsdfs)
add_subdirectory(subsurface)
add_subdirectory(textures)

# ------------------------------------------------------------------------------
# CUI/GUI environments
# ------------------------------------------------------------------------------
if (SPICA_BUILD_MAIN)
    add_subdirectory(spica)
endif()
