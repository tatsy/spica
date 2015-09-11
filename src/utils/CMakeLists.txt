configure_file(${CMAKE_CURRENT_LIST_DIR}/spica_dirs.h.in
               ${CMAKE_CURRENT_LIST_DIR}/spica_dirs.h
               @ONLY)

 set(SOURCES ${SOURCES}
     ${CMAKE_CURRENT_LIST_DIR}/vbo.cc
     ${CMAKE_CURRENT_LIST_DIR}/vector3d.cc
     ${CMAKE_CURRENT_LIST_DIR}/vector3d_avx.cc
     ${CMAKE_CURRENT_LIST_DIR}/color.cc
     ${CMAKE_CURRENT_LIST_DIR}/image.cc
     ${CMAKE_CURRENT_LIST_DIR}/sampler.cc
     PARENT_SCOPE)

 set(HEADERS ${HEADERS}
     ${CMAKE_CURRENT_LIST_DIR}/common.h
     ${CMAKE_CURRENT_LIST_DIR}/vbo.h
     ${CMAKE_CURRENT_LIST_DIR}/path.h
     ${CMAKE_CURRENT_LIST_DIR}/stack.h
     ${CMAKE_CURRENT_LIST_DIR}/uncopyable.h
     ${CMAKE_CURRENT_LIST_DIR}/vector3d.h
     ${CMAKE_CURRENT_LIST_DIR}/color.h
     ${CMAKE_CURRENT_LIST_DIR}/image.h
     ${CMAKE_CURRENT_LIST_DIR}/sampler.h
     ${CMAKE_CURRENT_LIST_DIR}/timer.h
     ${CMAKE_CURRENT_LIST_DIR}/axis_comparable.h
     ${CMAKE_CURRENT_LIST_DIR}/kdtree.h
     ${CMAKE_CURRENT_LIST_DIR}/kdtree_detail.h
     ${CMAKE_CURRENT_LIST_DIR}/random_queue.h
     ${CMAKE_CURRENT_LIST_DIR}/hash_grid.h
     ${CMAKE_CURRENT_LIST_DIR}/hash_grid_detail.h
     PARENT_SCOPE)
