#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PREDEFINED_SCENES_H_
#define _SPICA_PREDEFINED_SCENES_H_

#include <vector>
#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"

namespace spica {


    SPICA_EXPORTS void cornellBox(Scene* scene,
                                  std::shared_ptr<Camera>* camera,
                                  const Point2i& resolution,
                                  std::vector<Triangle>* tris = nullptr,
                                  std::vector<Spectrum>* Kd = nullptr);

    SPICA_EXPORTS void cornellBoxBunny(Scene* scene, Camera* camera, const int width, const int height);
   
    SPICA_EXPORTS void cornellBoxOcclusion(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_EXPORTS void cornellBoxDragon(Scene* scene, Camera* camera, const int width, const int height);
    
    SPICA_EXPORTS void kittenBox(Scene* scene, Camera* camera, const int width, const int height);

    SPICA_EXPORTS void kittenEnvmap(Scene* scene,
                                    std::shared_ptr<Camera>* camera,
                                    const Point2i& resolution,
                                    std::vector<Triangle>* tris = nullptr,
                                    std::vector<Spectrum>* Kd = nullptr);

}

#endif  // _SPICA_CORNELL_BOX_H_
