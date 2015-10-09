/*!
  @mainpage  
  The cross-platform rendering engine by modern C++
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_H_
#define _SPICA_H_

#include "../src/utils/common.h"
#include "../src/utils/vbo.h"
#include "../src/utils/path.h"
#include "../src/utils/timer.h"
#include "../src/utils/vector2d.h"
#include "../src/utils/vector3d.h"
#include "../src/utils/sampler.h"
#include "../src/utils/spica_dirs.h"

#include "../src/geometry/geometry.h"

#include "../src/renderer/brdf.h"
#include "../src/renderer/bssrdf.h"
#include "../src/renderer/render_parameters.h"

#include "../src/random/random.h"
#include "../src/random/halton.h"

// --------------------------------------------
// Lights
// --------------------------------------------

#include "../src/light/lighting.h"
#include "../src/light/area_light.h"
#include "../src/light/envmap.h"

// --------------------------------------------
// Cameras
// --------------------------------------------

#include "../src/camera/camera.h"
#include "../src/camera/orthographic_camera.h"
#include "../src/camera/perspective_camera.h"
#include "../src/camera/dof_camera.h"

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

#include "../src/scenes/scene.h"
#include "../src/scenes/predefined_scenes.h"
#include "../src/scenes/yaml_parser.h"
#include "../src/scenes/scene_loader.h"

// --------------------------------------------
// Structures
// --------------------------------------------

#include "../src/structure/mesh.h"

#endif  // _SPICA_H_
