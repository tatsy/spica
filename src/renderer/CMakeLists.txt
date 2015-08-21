# Add source files
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/ray.cc
    ${CMAKE_CURRENT_LIST_DIR}/camera.cc
    ${CMAKE_CURRENT_LIST_DIR}/scene.cc
    ${CMAKE_CURRENT_LIST_DIR}/bsdf.cc
    ${CMAKE_CURRENT_LIST_DIR}/brdf.cc
    ${CMAKE_CURRENT_LIST_DIR}/bssrdf.cc
    ${CMAKE_CURRENT_LIST_DIR}/envmap.cc
    ${CMAKE_CURRENT_LIST_DIR}/renderer_helper.cc
    ${CMAKE_CURRENT_LIST_DIR}/pathtrace.cc
    ${CMAKE_CURRENT_LIST_DIR}/bdpt.cc
    ${CMAKE_CURRENT_LIST_DIR}/mlt.cc
    ${CMAKE_CURRENT_LIST_DIR}/photon_map.cc
    ${CMAKE_CURRENT_LIST_DIR}/ppmprob.cc
    ${CMAKE_CURRENT_LIST_DIR}/sppm.cc
#    ${CMAKE_CURRENT_LIST_DIR}/subsurface_integrator.cc
#    ${CMAKE_CURRENT_LIST_DIR}/subsurface_sppm_renderer.cc
    PARENT_SCOPE)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/renderer_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/renderer_constants.h
    ${CMAKE_CURRENT_LIST_DIR}/render_parameters.h
    ${CMAKE_CURRENT_LIST_DIR}/ray.h
    ${CMAKE_CURRENT_LIST_DIR}/camera.h
    ${CMAKE_CURRENT_LIST_DIR}/scene.h
    ${CMAKE_CURRENT_LIST_DIR}/bsdf.h
    ${CMAKE_CURRENT_LIST_DIR}/brdf.h
    ${CMAKE_CURRENT_LIST_DIR}/bssrdf.h
    ${CMAKE_CURRENT_LIST_DIR}/envmap.h
    ${CMAKE_CURRENT_LIST_DIR}/renderer_helper.h
    ${CMAKE_CURRENT_LIST_DIR}/pathtrace.h
    ${CMAKE_CURRENT_LIST_DIR}/bdpt.h
    ${CMAKE_CURRENT_LIST_DIR}/mlt.h
    ${CMAKE_CURRENT_LIST_DIR}/photon_map.h
    ${CMAKE_CURRENT_LIST_DIR}/ppmprob.h
    ${CMAKE_CURRENT_LIST_DIR}/sppm.h
#    ${CMAKE_CURRENT_LIST_DIR}/subsurface_integrator.h
#    ${CMAKE_CURRENT_LIST_DIR}/subsurface_sppm_renderer.h
    PARENT_SCOPE)
