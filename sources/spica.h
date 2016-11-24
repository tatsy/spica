#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_H_
#define _SPICA_H_

#include "../sources/core/common.h"
#include "../sources/core/parallel.h"
#include "../sources/core/engine.h"
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
