set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/scene.h
#    ${CMAKE_CURRENT_LIST_DIR}/predefined_scenes.h
    # ${CMAKE_CURRENT_LIST_DIR}/scene_loader.h
#    ${CMAKE_CURRENT_LIST_DIR}/vertex_data.h
#    ${CMAKE_CURRENT_LIST_DIR}/triangle_data.h
#    ${CMAKE_CURRENT_LIST_DIR}/yaml_parser.h
    PARENT_SCOPE)

set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/scene.cc
#    ${CMAKE_CURRENT_LIST_DIR}/predefined_scenes.cc
    # ${CMAKE_CURRENT_LIST_DIR}/scene_loader.cc
#    ${CMAKE_CURRENT_LIST_DIR}/yaml_parser.cc
    PARENT_SCOPE)
