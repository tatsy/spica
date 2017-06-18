set(SPICA_TARGET_NAME spica_renderer CACHE INTERNAL "")
set(BUILD_TARGET ${SPICA_TARGET_NAME})

set(SOURCE_FILES "spica.h")


# Process subdirectories
add_subdirectory(core)
add_subdirectory(render)
add_subdirectory(cameras)
add_subdirectory(lights)
add_subdirectory(accelerators)
add_subdirectory(integrators)
add_subdirectory(shapes)

# process_subdirectory(image)
# process_subdirectory(filter)
# process_subdirectory(shape)
# process_subdirectory(medium)
# process_subdirectory(random)
# process_subdirectory(material)
# process_subdirectory(texture)

# Build viewer
if (BUILD_SPICA_GUI)
  add_subdirectory(viewer)
endif()

if (BUILD_SPICA_MAIN)
  add_subdirectory(main)
endif()
