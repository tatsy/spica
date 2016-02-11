set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/random.cc
    ${CMAKE_CURRENT_LIST_DIR}/halton.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/random_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/random.h
    ${CMAKE_CURRENT_LIST_DIR}/halton.h
    ${CMAKE_CURRENT_LIST_DIR}/sampler.h
    PARENT_SCOPE)
