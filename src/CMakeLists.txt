if(MSVC)
  find_package(OpenMP)
  if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

add_subdirectory(renderer)
add_subdirectory(geometry)
add_subdirectory(utils)

set(SOURCES ${SOURCES})
set(HEADERS ${HEADERS} ../include/spica.h)

add_library(spica_renderer SHARED ${SOURCES} ${HEADERS})
