# Add source files
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/Ray.cc
    ${CMAKE_CURRENT_LIST_DIR}/Scene.cc
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.cc
    ${CMAKE_CURRENT_LIST_DIR}/RendererBase.cc
    ${CMAKE_CURRENT_LIST_DIR}/BPTRenderer.cc
    PARENT_SCOPE)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/Ray.h
    ${CMAKE_CURRENT_LIST_DIR}/Scene.h
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.h
    ${CMAKE_CURRENT_LIST_DIR}/RendererBase.h
    ${CMAKE_CURRENT_LIST_DIR}/BPTRenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/material.h
    PARENT_SCOPE)
