macro(add_filter)
  add_spica_plugin(${ARGN} TYPE filter)
endmacro()

add_filter(box box.cc box.h)
add_filter(tent tent.cc tent.h)
add_filter(gaussian gaussian.cc gaussian.h)
