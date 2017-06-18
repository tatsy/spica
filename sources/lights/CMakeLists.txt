macro(add_light)
  add_spica_plugin(${ARGN} TYPE light)
endmacro()

add_light(area   area.cc area.h)
add_light(envmap envmap.cc envmap.h)
