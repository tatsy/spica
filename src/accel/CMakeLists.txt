set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/kd_tree_accel.cc
    ${CMAKE_CURRENT_LIST_DIR}/qbvh_accel.cc
    PARENT_SCOPE)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/accel_base.h
    ${CMAKE_CURRENT_LIST_DIR}/kd_tree_accel.h
    ${CMAKE_CURRENT_LIST_DIR}/qbvh_accel.h
    PARENT_SCOPE)
