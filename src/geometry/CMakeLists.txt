set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/plane.cc
    ${CMAKE_CURRENT_LIST_DIR}/primitive.cc
    ${CMAKE_CURRENT_LIST_DIR}/quad.cc
    ${CMAKE_CURRENT_LIST_DIR}/sphere.cc
    ${CMAKE_CURRENT_LIST_DIR}/triangle.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/plane.h
    ${CMAKE_CURRENT_LIST_DIR}/primitive.h
    ${CMAKE_CURRENT_LIST_DIR}/quad.h
    ${CMAKE_CURRENT_LIST_DIR}/sphere.h
    ${CMAKE_CURRENT_LIST_DIR}/triangle.h
    PARENT_SCOPE)
