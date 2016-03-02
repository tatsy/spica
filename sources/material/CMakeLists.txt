set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/material.cc
    ${CMAKE_CURRENT_LIST_DIR}/lambert.cc
    ${CMAKE_CURRENT_LIST_DIR}/mirror.cc
    ${CMAKE_CURRENT_LIST_DIR}/metal.cc
    ${CMAKE_CURRENT_LIST_DIR}/glass.cc
    ${CMAKE_CURRENT_LIST_DIR}/subsurface.cc
    ${CMAKE_CURRENT_LIST_DIR}/kdsubsurface.cc
    ${CMAKE_CURRENT_LIST_DIR}/translucent.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/spica_material.h
    ${CMAKE_CURRENT_LIST_DIR}/lambert.h
    ${CMAKE_CURRENT_LIST_DIR}/mirror.h
    ${CMAKE_CURRENT_LIST_DIR}/metal.h
    ${CMAKE_CURRENT_LIST_DIR}/glass.h
    ${CMAKE_CURRENT_LIST_DIR}/material.h
    ${CMAKE_CURRENT_LIST_DIR}/subsurface.h
    ${CMAKE_CURRENT_LIST_DIR}/kdsubsurface.h
    ${CMAKE_CURRENT_LIST_DIR}/translucent.h
    PARENT_SCOPE)
