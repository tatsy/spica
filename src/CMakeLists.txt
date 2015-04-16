add_subdirectory(renderer)
add_subdirectory(geometry)
add_subdirectory(utils)

set(SOURCES ${SOURCES})
set(HEADERS ${HEADERS} ../include/spica.h)

add_library(spica_renderer SHARED ${SOURCES} ${HEADERS})
