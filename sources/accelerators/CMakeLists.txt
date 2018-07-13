macro(add_accelerator)
  add_spica_plugin(${ARGN} TYPE accelerator)
endmacro()

add_accelerator(bvh    bvh.cc bvh.h)
