set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/film.cc
    ${CMAKE_CURRENT_LIST_DIR}/perspective_camera.cc
    ${CMAKE_CURRENT_LIST_DIR}/orthographic_camera.cc
    ${CMAKE_CURRENT_LIST_DIR}/dof_camera.cc
    ${CMAKE_CURRENT_LIST_DIR}/camera.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/film.h
    ${CMAKE_CURRENT_LIST_DIR}/camera_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/perspective_camera.h
    ${CMAKE_CURRENT_LIST_DIR}/orthographic_camera.h
    ${CMAKE_CURRENT_LIST_DIR}/dof_camera.h
    ${CMAKE_CURRENT_LIST_DIR}/camera.h
    PARENT_SCOPE)
