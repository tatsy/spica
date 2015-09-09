#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_H_
#define _SPICA_H_

#include "../src/utils/common.h"
#include "../src/utils/path.h"
#include "../src/utils/timer.h"
#include "../src/utils/vector3d.h"
#include "../src/utils/sampler.h"
#include "../src/utils/spica_dirs.h"

#include "../src/geometry/geometry.h"

#include "../src/renderer/brdf.h"
#include "../src/renderer/bssrdf.h"
#include "../src/renderer/scene.h"
#include "../src/renderer/camera.h"
#include "../src/renderer/render_parameters.h"

#include "../src/random/random.h"
#include "../src/random/halton.h"

// --------------------------------------------
// Renderers
// --------------------------------------------

#include "../src/renderer/pathtrace.h"
#include "../src/renderer/bdpt.h"
#include "../src/renderer/mlt.h"
#include "../src/renderer/ppmprob.h"
#include "../src/renderer/sppm.h"
#include "../src/renderer/subsurface_sppm_renderer.h"

// --------------------------------------------
// Scenes
// --------------------------------------------

#include "../src/scenes/predefined_scenes.h"
#include "../src/scenes/yaml_parser.h"
#include "../src/scenes/scene_loader.h"

#endif  // _SPICA_H_
