macro(add_integrator)
    add_spica_plugin(${ARGN} TYPE integrator)
endmacro()

add_integrator(path path/path.cc path/path.h)
add_integrator(bdpt bdpt/bdpt.cc bdpt/bdpt.h bidirectional.cc bidirectional.cc)
add_integrator(gdpt gdpt/gdpt.cc gdpt/gdpt.h gdpt/gdptfilm.cc gdpt/gdptfilm.h
               LINK_LIBRARIES ${SPICA_DEPENDENCY_LIBRARIES})
add_integrator(pssmlt pssmlt/pssmlt.cc pssmlt/pssmlt.h)
add_integrator(sppm sppm/sppm.cc sppm/sppm.h)
add_integrator(directlighting directlighting/directlighting.cc directlighting/directlighting.h)
add_integrator(hierarchical hierarchical/hierarchical.cc hierarchical/hierarchical.h photon_map.cc photon_map.h)
