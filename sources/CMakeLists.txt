set(SPICA_TARGET_NAME spica_renderer CACHE INTERNAL "")
set(BUILD_TARGET ${SPICA_TARGET_NAME})

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
  # include_directories(${CMAKE_CURRENT_LIST_DIR}/${dirname})
  file(GLOB FILTER_FILES "${dirname}/*.cc" "${dirname}/*.h")

  if (MSVC)
    source_group(${dirname} FILES ${FILTER_FILES})
  endif()
endfunction()

# Process subdirectories
process_subdirectory(core)
process_subdirectory(math)
process_subdirectory(image)
process_subdirectory(filter)
process_subdirectory(integrator)
process_subdirectory(shape)
process_subdirectory(medium)
process_subdirectory(bxdf)
process_subdirectory(random)
process_subdirectory(light)
process_subdirectory(camera)
process_subdirectory(scenes)
process_subdirectory(accel)
process_subdirectory(material)
process_subdirectory(texture)

add_library(${BUILD_TARGET} SHARED ${ALL_SOURCES} ${ALL_HEADERS})
set(SPICA_LIBS ${LIB_PREFIX}${BUILD_TARGET}${LIB_SUFFIX} CACHE INTERNAL "")

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
endif()

# Build viewer
add_subdirectory(viewer)
