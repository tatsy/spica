macro(add_integrator)
    add_spica_plugin(${ARGN} TYPE integrator)
endmacro()

add_integrator(path path/path.cc path/path.h)
add_integrator(volpath volpath/volpath.cc volpath/volpath.h)

add_integrator(bdpt bdpt/bdpt.cc bdpt/bdpt.h bidirectional.cc bidirectional.h)
add_integrator(gdpt gdpt/gdpt.cc gdpt/gdpt.h gdpt/gdptfilm.cc gdpt/gdptfilm.h
               LINK_LIBRARIES ${SPICA_DEPENDENCY_LIBRARIES})

add_integrator(vcmups vcmups/vcmups.cc vcmups/vcmups.h
               bidirectional.cc bidirectional.h
               photonmap.cc photonmap.h
               LINK_LIBRARIES ${SPICA_DEPENDENCY_LIBRARIES})

add_integrator(pssmlt pssmlt/pssmlt.cc pssmlt/pssmlt.h
               primarysample.cc primarysample.h)
add_integrator(mmlt mmlt/mmlt.cc mmlt/mmlt.h
               bidirectional.cc bidirectional.h
               primarysample.cc primarysample.h)

add_integrator(photonmapper photonmapper/photonmapper.cc photonmapper/photonmapper.h
               photonmap.cc photonmap.h)
add_integrator(sppm sppm/sppm.cc sppm/sppm.h)
add_integrator(volphoto volphoto/volphoto.cc volphoto/volphoto.h
               photonmap.cc photonmap.h)

add_integrator(directlighting directlighting/directlighting.cc directlighting/directlighting.h)
add_integrator(hierarchical hierarchical/hierarchical.cc hierarchical/hierarchical.h photonmap.cc photonmap.h)
