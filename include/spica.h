#ifndef SPICA_H_
#define SPICA_H_

#include "../src/utils/common.h"
#include "../src/utils/timer.h"
#include "../src/utils/vector3.h"
#include "../src/utils/sampler.h"

#include "../src/geometry/geometry.h"

#include "../src/renderer/scene.h"
#include "../src/renderer/camera.h"


// --------------------------------------------
// Renderers
// --------------------------------------------

#include "../src/renderer/pt_renderer.h"
#include "../src/renderer/bpt_renderer.h"
#include "../src/renderer/mlt_renderer.h"
#include "../src/renderer/photon_mapping.h"
#include "../src/renderer/progressive_photonmap.h"
#include "../src/renderer/sss_renderer.h"

// --------------------------------------------
// Scenes
// --------------------------------------------

#include "../src/scenes/cornell_box.h"

#endif  // SPICA_H_
