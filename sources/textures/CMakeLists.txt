macro(add_texture)
  add_spica_plugin(${ARGN} TYPE texture)
endmacro()

add_texture(checkerboard checkerboard.cc checkerboard.h)
