macro(add_medium)
  add_spica_plugin(${ARGN} TYPE medium)
endmacro()

add_medium(homogeneous homogeneous.cc homogeneous.h)
