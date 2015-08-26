set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/bbox.cc
    ${CMAKE_CURRENT_LIST_DIR}/plane.cc
    ${CMAKE_CURRENT_LIST_DIR}/quad.cc
    ${CMAKE_CURRENT_LIST_DIR}/disk.cc
    ${CMAKE_CURRENT_LIST_DIR}/sphere.cc
    ${CMAKE_CURRENT_LIST_DIR}/triangle.cc
    ${CMAKE_CURRENT_LIST_DIR}/trimesh.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/geometry.h
    ${CMAKE_CURRENT_LIST_DIR}/bbox.h
    ${CMAKE_CURRENT_LIST_DIR}/plane.h
    ${CMAKE_CURRENT_LIST_DIR}/geometry_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/quad.h
    ${CMAKE_CURRENT_LIST_DIR}/disk.h
    ${CMAKE_CURRENT_LIST_DIR}/sphere.h
    ${CMAKE_CURRENT_LIST_DIR}/triangle.h
    ${CMAKE_CURRENT_LIST_DIR}/trimesh.h
    PARENT_SCOPE)
