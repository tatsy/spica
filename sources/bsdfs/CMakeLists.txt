macro(add_bsdf)
  add_spica_plugin(${ARGN} TYPE bsdf)
endmacro()

add_bsdf(diffuse         diffuse.cc diffuse.h)
add_bsdf(conductor       conductor.cc conductor.h)
add_bsdf(roughconductor  roughconductor.cc roughconductor.h)
add_bsdf(dielectric      dielectric.cc dielectric.h)
add_bsdf(roughdielectric roughdielectric.cc roughdielectric.h)
add_bsdf(plastic         plastic.cc plastic.h)
add_bsdf(roughplastic    roughplastic.cc roughplastic.h)
add_bsdf(msconductor     microsurface/msconductor.cc microsurface/msconductor.h
                         microsurface/microsurface.cc microsurface/microsurface.h)
add_bsdf(msdielectric    microsurface/msdielectric.cc microsurface/msdielectric.h
                         microsurface/microsurface.cc microsurface/microsurface.h)
