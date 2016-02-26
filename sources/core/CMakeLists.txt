configure_file(${CMAKE_CURRENT_LIST_DIR}/spica_dirs.h.in
               ${CMAKE_CURRENT_LIST_DIR}/spica_dirs.h
               @ONLY)

 set(SOURCES ${SOURCES}
     ${CMAKE_CURRENT_LIST_DIR}/memory.cc
     ${CMAKE_CURRENT_LIST_DIR}/spectrum.cc
     ${CMAKE_CURRENT_LIST_DIR}/sampling.cc
     ${CMAKE_CURRENT_LIST_DIR}/primitive.cc
     ${CMAKE_CURRENT_LIST_DIR}/interaction.cc
     ${CMAKE_CURRENT_LIST_DIR}/interpolation.cc
     ${CMAKE_CURRENT_LIST_DIR}/ray.cc
     PARENT_SCOPE)

 set(HEADERS ${HEADERS}
     ${CMAKE_CURRENT_LIST_DIR}/common.h
     ${CMAKE_CURRENT_LIST_DIR}/memory.h
     ${CMAKE_CURRENT_LIST_DIR}/forward_decl.h
     ${CMAKE_CURRENT_LIST_DIR}/triplet.h
     ${CMAKE_CURRENT_LIST_DIR}/point2d.h
     ${CMAKE_CURRENT_LIST_DIR}/point2d_detail.h
     ${CMAKE_CURRENT_LIST_DIR}/point3d.h
     ${CMAKE_CURRENT_LIST_DIR}/point3d_detail.h
     ${CMAKE_CURRENT_LIST_DIR}/bounds3d.h
     ${CMAKE_CURRENT_LIST_DIR}/bounds3d_detail.h
     ${CMAKE_CURRENT_LIST_DIR}/normal3d.h
     ${CMAKE_CURRENT_LIST_DIR}/normal3d_detail.h
     ${CMAKE_CURRENT_LIST_DIR}/path.h
     ${CMAKE_CURRENT_LIST_DIR}/rect.h
     ${CMAKE_CURRENT_LIST_DIR}/stack.h
     ${CMAKE_CURRENT_LIST_DIR}/uncopyable.h
     ${CMAKE_CURRENT_LIST_DIR}/spectrum.h
     ${CMAKE_CURRENT_LIST_DIR}/sampling.h
     ${CMAKE_CURRENT_LIST_DIR}/primitive.h
     ${CMAKE_CURRENT_LIST_DIR}/interaction.h
     ${CMAKE_CURRENT_LIST_DIR}/interpolation.h
     ${CMAKE_CURRENT_LIST_DIR}/ray.h
     ${CMAKE_CURRENT_LIST_DIR}/timer.h
     ${CMAKE_CURRENT_LIST_DIR}/axis_comparable.h
     ${CMAKE_CURRENT_LIST_DIR}/kdtree.h
     ${CMAKE_CURRENT_LIST_DIR}/kdtree_detail.h
     ${CMAKE_CURRENT_LIST_DIR}/random_queue.h
     ${CMAKE_CURRENT_LIST_DIR}/hash_grid.h
     ${CMAKE_CURRENT_LIST_DIR}/hash_grid_detail.h
     PARENT_SCOPE)
