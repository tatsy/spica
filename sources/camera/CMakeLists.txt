set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/camera.cc
    ${CMAKE_CURRENT_LIST_DIR}/perspective.cc
    ${CMAKE_CURRENT_LIST_DIR}/orthographic.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/spica_camera.h
    ${CMAKE_CURRENT_LIST_DIR}/camera.h
    ${CMAKE_CURRENT_LIST_DIR}/perspective.h
    ${CMAKE_CURRENT_LIST_DIR}/orthographic.h
    PARENT_SCOPE)
