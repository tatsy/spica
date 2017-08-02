macro(add_camera)
  add_spica_plugin(${ARGN} TYPE camera)
endmacro()

add_camera(orthographic orthographic.cc orthographic.h)
add_camera(perspective  perspective.cc perspective.h)
