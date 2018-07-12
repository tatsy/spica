#ifdef _MSC_VER
#pragma once
#endif

<<<<<<< HEAD
#ifndef _SPICA_HEADER_H_
#define _SPICA_HEADER_H_

#include "core/common.h"
#include "core/parallel.h"
#include "core/exception.h"
#include "core/timer.h"
#include "core/memory.h"
#include "core/sampling.h"
#include "core/point2d.h"
#include "core/point3d.h"
#include "core/bounds2d.h"
#include "core/bounds3d.h"
#include "core/triplet.h"
#include "core/interaction.h"
#include "core/random_queue.h"
#include "core/renderparams.h"
#include "core/shape.h"
#include "core/bsphere.h"
#include "core/triangle.h"
#include "core/primitive.h"
#include "core/random.h"
#include "core/accelerator.h"

#include "core/bsdf.h"
#include "core/bxdf.h"
#include "core/bssrdf.h"
#include "core/fresnel.h"
#include "core/microfacet.h"

#include "core/math.h"
#include "core/quaternion.h"

#include "core/image.h"
#include "core/film.h"
#include "core/tmo.h"
#include "core/birateral.h"

#include "core/filter.h"
#include "core/camera.h"
#include "core/integrator.h"
#include "core/texture.h"
#include "core/scene.h"

#endif  // _SPICA_HEADER_H_
=======
#ifndef _SPICA_H_
#define _SPICA_H_

#include "../sources/core/common.h"
#include "../sources/core/parallel.h"
#include "../sources/core/path.h"
#include "../sources/core/timer.h"
#include "../sources/core/memory.h"
#include "../sources/core/sampling.h"
#include "../sources/core/spica_dirs.h"
#include "../sources/core/point2d.h"
#include "../sources/core/point3d.h"
#include "../sources/core/bounds2d.h"
#include "../sources/core/bounds3d.h"
#include "../sources/core/triplet.h"
#include "../sources/core/interaction.h"
#include "../sources/core/random_queue.h"
#include "../sources/core/renderparams.h"

#include "../sources/shape/shape.h"

#include "../sources/random/random.h"
#include "../sources/random/halton.h"



// --------------------------------------------
// Accelerators
// --------------------------------------------
#include "../sources/accelerator/spica_accelerator.h"

// --------------------------------------------
// BxDF
// --------------------------------------------
#include "../sources/bxdf/bsdf.h"
#include "../sources/bxdf/bxdf.h"
#include "../sources/bxdf/bssrdf.h"
#include "../sources/bxdf/fresnel.h"
#include "../sources/bxdf/microfacet.h"

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
// Integrators
// --------------------------------------------

#include "../sources/integrator/spica_integrator.h"

// --------------------------------------------
// Textures
// --------------------------------------------

#include "../sources/texture/spica_texture.h"

// --------------------------------------------
// Materials
// --------------------------------------------

#include "../sources/material/spica_material.h"

// --------------------------------------------
// Scenes
// --------------------------------------------

#include "../sources/scenes/scene.h"

// --------------------------------------------
// Viewer
// --------------------------------------------

#include "../sources/viewer/vbo.h"

#endif  // _SPICA_H_
>>>>>>> 6c734230b0e5cd1f2dd3f640096ad004c8ace723
