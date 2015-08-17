#ifndef SPICA_H_
#define SPICA_H_

#include "../src/utils/common.h"
#include "../src/utils/timer.h"
#include "../src/utils/vector3d.h"
#include "../src/utils/sampler.h"
#include "../src/utils/vbo.h"

#include "../src/geometry/geometry.h"

#include "../src/renderer/brdf.h"
#include "../src/renderer/bssrdf.h"
#include "../src/renderer/scene.h"
#include "../src/renderer/camera.h"

#include "../src/random/random.h"
#include "../src/random/halton.h"

// --------------------------------------------
// Renderers
// --------------------------------------------

#include "../src/renderer/pt_renderer.h"
#include "../src/renderer/bpt_renderer.h"
#include "../src/renderer/mlt_renderer.h"
#include "../src/renderer/photon_mapping.h"
#include "../src/renderer/progressive_photonmap.h"
#include "../src/renderer/subsurface_sppm_renderer.h"

// --------------------------------------------
// Scenes
// --------------------------------------------

#include "../src/scenes/predefined_scenes.h"

#endif  // SPICA_H_
