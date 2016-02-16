set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/shape.cc
    ${CMAKE_CURRENT_LIST_DIR}/disk.cc
    ${CMAKE_CURRENT_LIST_DIR}/sphere.cc
    ${CMAKE_CURRENT_LIST_DIR}/triangle.cc
    ${CMAKE_CURRENT_LIST_DIR}/meshio.cc
    ${CMAKE_CURRENT_LIST_DIR}/visibility_tester.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/spica_shape.h
    ${CMAKE_CURRENT_LIST_DIR}/shape.h
    ${CMAKE_CURRENT_LIST_DIR}/disk.h
    ${CMAKE_CURRENT_LIST_DIR}/sphere.h
    ${CMAKE_CURRENT_LIST_DIR}/triangle.h
    ${CMAKE_CURRENT_LIST_DIR}/meshio.h
    ${CMAKE_CURRENT_LIST_DIR}/visibility_tester.h
    PARENT_SCOPE)
