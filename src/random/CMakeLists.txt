set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/random.cc
    ${CMAKE_CURRENT_LIST_DIR}/halton.cc
    ${CMAKE_CURRENT_LIST_DIR}/primary_sampler.cc
    PARENT_SCOPE)

set(HEADERS ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/random.h
    ${CMAKE_CURRENT_LIST_DIR}/halton.h
    ${CMAKE_CURRENT_LIST_DIR}/random_base.h
    ${CMAKE_CURRENT_LIST_DIR}/primary_sampler.h
    PARENT_SCOPE)
