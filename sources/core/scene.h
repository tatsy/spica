#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_H_
#define _SPICA_SCENE_H_

#include <vector>
#include <memory>

#include "core/core.hpp"
#include "core/common.h"

#include "core/bounds3d.h"
#include "core/light.h"
#include "core/accelerator.h"

namespace spica {

/**
 * Scene provides the interface for scene graph.
 */
class SPICA_EXPORTS Scene : private Uncopyable {
public:
    Scene();
    Scene(const std::shared_ptr<Accelerator>& aggregate,
            const std::vector<std::shared_ptr<Light> >& lights);
    Scene(Scene&& scene);

    virtual ~Scene();

    Scene& operator=(Scene&& scene);

    bool intersect(Ray& ray, SurfaceInteraction* isect) const;
    bool intersect(Ray& ray) const;
    bool intersectTr(Ray& ray, Sampler& sampler, SurfaceInteraction* isect,
                     Spectrum* tr) const;

    inline const Bounds3d& worldBound() const { return worldBound_; }
    inline const std::vector<std::shared_ptr<Primitive>>& primitives() const {
        return aggregate_->primitives();
    }
    inline const std::vector<std::shared_ptr<Light> >& lights() const {
        return lights_;
    }

private:
    std::shared_ptr<Accelerator> aggregate_;
    std::vector<std::shared_ptr<Light> > lights_;
    Bounds3d worldBound_;

};  // class Scene

}  // namespace spica

#endif  // _SPICA_SCENE_H_
