macro(add_sampler)
  add_spica_plugin(${ARGN} TYPE sampler)
endmacro()

add_sampler(halton halton.cc halton.h)
add_sampler(independent independent.cc independent.h)
