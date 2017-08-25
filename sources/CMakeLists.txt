# Process subdirectories
add_subdirectory(core)

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

# process_subdirectory(image)
# process_subdirectory(filter)
# process_subdirectory(medium)
# process_subdirectory(texture)

# Build viewer
if (SPICA_BUILD_GUI)
  add_subdirectory(spicagui)
endif()

if (SPICA_BUILD_MAIN)
  add_subdirectory(spica)
endif()
