set(SPICA_TARGET_NAME spica_renderer CACHE INTERNAL "")
set(BUILD_TARGET ${SPICA_TARGET_NAME})

set(SOURCE_FILES "spica.h")


# Process subdirectories
add_subdirectory(core)
add_subdirectory(render)
add_subdirectory(cameras)
# process_subdirectory(image)
# process_subdirectory(filter)
# process_subdirectory(integrator)
# process_subdirectory(shape)
# process_subdirectory(medium)
# process_subdirectory(bxdf)
# process_subdirectory(random)
# process_subdirectory(light)
# process_subdirectory(camera)
# process_subdirectory(scenes)
# process_subdirectory(accelerator)
# process_subdirectory(material)
# process_subdirectory(texture)

# Build viewer
if (BUILD_SPICA_GUI)
  add_subdirectory(viewer)
endif()

if (BUILD_SPICA_MAIN)
  add_subdirectory(main)
endif()
