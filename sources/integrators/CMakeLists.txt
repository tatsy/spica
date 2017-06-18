macro(add_integrator)
  add_spica_plugin(${ARGN} TYPE integrator)
endmacro()

add_integrator(path path/path.cc path/path.h)
