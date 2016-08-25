set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/texture.cc
    ${CMAKE_CURRENT_LIST_DIR}/constant.cc
    ${CMAKE_CURRENT_LIST_DIR}/imagemap.cc
    ${CMAKE_CURRENT_LIST_DIR}/floatmap.cc
    ${CMAKE_CURRENT_LIST_DIR}/uv.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/spica_texture.h
    ${CMAKE_CURRENT_LIST_DIR}/constant.h
    ${CMAKE_CURRENT_LIST_DIR}/imagemap.h
    ${CMAKE_CURRENT_LIST_DIR}/floatmap.h
    ${CMAKE_CURRENT_LIST_DIR}/texture.h
    ${CMAKE_CURRENT_LIST_DIR}/uv.h
    PARENT_SCOPE)
