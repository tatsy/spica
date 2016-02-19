# Add source files
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/integrator.cc
    ${CMAKE_CURRENT_LIST_DIR}/mis.cc
#    ${CMAKE_CURRENT_LIST_DIR}/renderer_helper.cc
    ${CMAKE_CURRENT_LIST_DIR}/pathtrace.cc
#    ${CMAKE_CURRENT_LIST_DIR}/bdpt.cc
#    ${CMAKE_CURRENT_LIST_DIR}/mlt.cc
#    ${CMAKE_CURRENT_LIST_DIR}/photon_map.cc
#    ${CMAKE_CURRENT_LIST_DIR}/ppmprob.cc
    ${CMAKE_CURRENT_LIST_DIR}/sppm.cc
#    ${CMAKE_CURRENT_LIST_DIR}/subsurface_integrator.cc
    PARENT_SCOPE)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/spica_integrator.h
    ${CMAKE_CURRENT_LIST_DIR}/integrator.h
    ${CMAKE_CURRENT_LIST_DIR}/mis.h
    ${CMAKE_CURRENT_LIST_DIR}/render_parameters.h
#    ${CMAKE_CURRENT_LIST_DIR}/renderer_helper.h
    ${CMAKE_CURRENT_LIST_DIR}/pathtrace.h
#    ${CMAKE_CURRENT_LIST_DIR}/bdpt.h
#    ${CMAKE_CURRENT_LIST_DIR}/mlt.h
#    ${CMAKE_CURRENT_LIST_DIR}/photon_map.h
#    ${CMAKE_CURRENT_LIST_DIR}/ppmprob.h
    ${CMAKE_CURRENT_LIST_DIR}/sppm.h
#    ${CMAKE_CURRENT_LIST_DIR}/subsurface_integrator.h
    PARENT_SCOPE)
