macro(add_shape)
  add_spica_plugin(${ARGN} TYPE shape)
endmacro()

add_shape(disk   disk.cc disk.h)
add_shape(sphere sphere.cc sphere.h)
