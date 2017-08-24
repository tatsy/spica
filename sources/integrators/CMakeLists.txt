macro(add_integrator)
  add_spica_plugin(${ARGN} TYPE integrator)
endmacro()

add_integrator(path path/path.cc path/path.h)
add_integrator(gdpt gdpt/gdpt.cc gdpt/gdpt.h gdpt/gdptfilm.cc gdpt/gdptfilm.h)
add_integrator(sppm sppm/sppm.cc sppm/sppm.h)
add_integrator(directlighting directlighting/directlighting.cc directlighting/directlighting.h)
