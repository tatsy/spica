# Add source files
list(APPEND SOURCE_FILES Random.h Random_detail.h)
list(APPEND SOURCE_FILES Vector3.h Vector3.cc)
list(APPEND SOURCE_FILES Sphere.h Sphere.cc)
list(APPEND SOURCE_FILES Ray.h Ray.cc)
list(APPEND SOURCE_FILES Scene.h Scene.cc)
list(APPEND SOURCE_FILES Renderer.h Renderer.cc)
list(APPEND SOURCE_FILES material.h)
list(APPEND SOURCE_FILES ../../include/rainy.h common.h)

add_library(rainy_renderer SHARED ${SOURCE_FILES})
