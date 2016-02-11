set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/image.cc
    ${CMAKE_CURRENT_LIST_DIR}/birateral.cc
    ${CMAKE_CURRENT_LIST_DIR}/tmo.cc
    ${CMAKE_CURRENT_LIST_DIR}/mipmap.cc
    PARENT_SCOPE)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/image.h
    ${CMAKE_CURRENT_LIST_DIR}/birateral.h
    ${CMAKE_CURRENT_LIST_DIR}/tmo.h
    ${CMAKE_CURRENT_LIST_DIR}/mipmap.h
    PARENT_SCOPE)
