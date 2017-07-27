macro(add_film)
  add_spica_plugin(${ARGN} TYPE film)
endmacro()

add_film(hdrfilm  hdrfilm.h hdrfilm.cc)
add_film(ldrfilm  ldrfilm.h ldrfilm.cc)
