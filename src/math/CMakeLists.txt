set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/quaternion.cc
    ${CMAKE_CURRENT_LIST_DIR}/matrix4x4.cc
    ${CMAKE_CURRENT_LIST_DIR}/transform.cc
    PARENT_SCOPE)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/spica_math.h
    ${CMAKE_CURRENT_LIST_DIR}/vector2d.h
    ${CMAKE_CURRENT_LIST_DIR}/vector2d_detail.h
    ${CMAKE_CURRENT_LIST_DIR}/vector3d.h
    ${CMAKE_CURRENT_LIST_DIR}/vector3d_detail.h
    ${CMAKE_CURRENT_LIST_DIR}/quaternion.h
    ${CMAKE_CURRENT_LIST_DIR}/matrix4x4.h
    ${CMAKE_CURRENT_LIST_DIR}/transform.h
    PARENT_SCOPE)
