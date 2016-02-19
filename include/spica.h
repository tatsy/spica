#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_H_
#define _SPICA_H_

#include "../sources/core/common.h"
// #include "../sources/core/vbo.h"
#include "../sources/core/path.h"
#include "../sources/core/timer.h"
#include "../sources/core/sampling.h"
#include "../sources/core/spica_dirs.h"
#include "../sources/core/rect.h"


#include "../sources/shape/shape.h"

#include "../sources/random/random.h"
#include "../sources/random/halton.h"

// --------------------------------------------
// Math
// --------------------------------------------

#include "../sources/math/spica_math.h"

// --------------------------------------------
// Image
// --------------------------------------------

#include "../sources/image/image.h"
#include "../sources/image/film.h"
#include "../sources/image/tmo.h"
#include "../sources/image/birateral.h"

// --------------------------------------------
// Filter
// --------------------------------------------

#include "../sources/filter/spica_filter.h"

// --------------------------------------------
// Lights
// --------------------------------------------

#include "../sources/light/area_light.h"
#include "../sources/light/envmap.h"

// --------------------------------------------
// Cameras
// --------------------------------------------

#include "../sources/camera/spica_camera.h"

// --------------------------------------------
// Renderers
// --------------------------------------------

#include "../sources/integrator/render_parameters.h"
#include "../sources/integrator/renderer_helper.h"

#include "../sources/integrator/pathtrace.h"
// #include "../sources/renderer/bdpt.h"
// #include "../sources/renderer/mlt.h"
// #include "../sources/renderer/ppmprob.h"
// #include "../sources/renderer/sppm.h"
// #include "../sources/renderer/subsurface_integrator.h"

// --------------------------------------------
// Scenes
// --------------------------------------------

#include "../sources/scenes/scene.h"
#include "../sources/scenes/predefined_scenes.h"
#include "../sources/scenes/yaml_parser.h"
#include "../sources/scenes/scene_loader.h"

#endif  // _SPICA_H_
