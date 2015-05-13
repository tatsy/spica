
configure_file(${CMAKE_CURRENT_LIST_DIR}/common.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/common.h
               @ONLY)

set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/vector3.cc
    ${CMAKE_CURRENT_LIST_DIR}/color.cc
    ${CMAKE_CURRENT_LIST_DIR}/image.cc
    ${CMAKE_CURRENT_LIST_DIR}/sampler.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/common.h
    ${CMAKE_CURRENT_LIST_DIR}/uncopyable.h
    ${CMAKE_CURRENT_LIST_DIR}/random.h
    ${CMAKE_CURRENT_LIST_DIR}/random_detail.h
    ${CMAKE_CURRENT_LIST_DIR}/vector3.h
    ${CMAKE_CURRENT_LIST_DIR}/color.h
    ${CMAKE_CURRENT_LIST_DIR}/image.h
    ${CMAKE_CURRENT_LIST_DIR}/sampler.h
    ${CMAKE_CURRENT_LIST_DIR}/timer.h
    ${CMAKE_CURRENT_LIST_DIR}/kdtree.h
    ${CMAKE_CURRENT_LIST_DIR}/kdtree_detail.h
    PARENT_SCOPE)
