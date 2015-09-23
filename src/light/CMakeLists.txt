set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/area_light.cc
    ${CMAKE_CURRENT_LIST_DIR}/envmap.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/light_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/area_light.h
    ${CMAKE_CURRENT_LIST_DIR}/envmap.h
    PARENT_SCOPE)
