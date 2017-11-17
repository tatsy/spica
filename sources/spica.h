#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_HEADER_H_
#define _SPICA_HEADER_H_

#include "core/common.h"
#include "core/parallel.h"
#include "core/exception.h"
#include "core/timer.h"
#include "core/memory.h"
#include "core/sampling.h"
#include "core/spica_dirs.h"
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
