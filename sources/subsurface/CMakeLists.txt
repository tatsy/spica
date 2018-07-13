macro(add_subsurface)
  add_spica_plugin(${ARGN} TYPE subsurface)
endmacro()

add_subsurface(dipole  dipole.cc dipole.h)
