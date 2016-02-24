set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/bsdf.cc
    ${CMAKE_CURRENT_LIST_DIR}/bxdf.cc
    ${CMAKE_CURRENT_LIST_DIR}/fresnel.cc
    ${CMAKE_CURRENT_LIST_DIR}/microfacet.cc
    # ${CMAKE_CURRENT_LIST_DIR}/bssrdf.cc
    PARENT_SCOPE)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/bsdf.h
    ${CMAKE_CURRENT_LIST_DIR}/bxdf.h
    ${CMAKE_CURRENT_LIST_DIR}/fresnel.h
    ${CMAKE_CURRENT_LIST_DIR}/microfacet.h
    # ${CMAKE_CURRENT_LIST_DIR}/bssrdf.h
    PARENT_SCOPE)
