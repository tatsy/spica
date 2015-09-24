if(MSVC)
  find_package(OpenMP)
  if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

set(ALL_SOURCES ${ALL_SOURCES})
set(ALL_HEADERS ${ALL_HEADERS} ../include/spica.h)

# Function to extract files from subdirectories
# and make file filters for MSVC
function (process_subdirectory dirname)
  set(SOURCES ${SOURCES})
  set(HEADERS ${HEADERS})
  add_subdirectory(${dirname})
  set(ALL_SOURCES ${ALL_SOURCES} ${SOURCES} PARENT_SCOPE)
  set(ALL_HEADERS ${ALL_HEADERS} ${HEADERS} PARENT_SCOPE)

  file(GLOB FILTER_FILES "${dirname}/*.cc" "${dirname}/*.h")
  if (MSVC)
    source_group(${dirname} FILES ${FILTER_FILES})
  endif()
endfunction()

# Process subdirectories
process_subdirectory(utils)
process_subdirectory(renderer)
process_subdirectory(geometry)
process_subdirectory(random)
process_subdirectory(light)
process_subdirectory(camera)
process_subdirectory(scenes)
process_subdirectory(accel)

add_library(spica_renderer SHARED ${ALL_SOURCES} ${ALL_HEADERS})

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set_property(TARGET spica_renderer APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
  configure_file(${SPICA_ROOT_DIRECTORY}/template/project.vcxproj.user.in "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/spica_renderer.vcxproj.user" @ONLY)
endif()

# Build viewer
add_subdirectory(viewer)
