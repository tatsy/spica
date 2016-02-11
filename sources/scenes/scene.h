#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_H_
#define _SPICA_SCENE_H_

#include <vector>
#include <memory>
#include <type_traits>

#include "../core/common.h"
#include "../core/uncopyable.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"
#include "../light/light.h"

namespace spica {

    /**
     * Scene provides the interface for scene graph.
     */
    class SPICA_EXPORTS Scene : private Uncopyable {
    public:
        /** The Scene constructor. */
        Scene();

        /** The Scene constructor (move). */
        Scene(Scene&& scene);

        /** The Scene destructor. */
        virtual ~Scene();

        /** Assignment operator. */
        Scene& operator=(Scene&& scene);

        /** Clear current scene. */
        void clear();

        /** Compute bounding sphere. */
        Sphere boundingSphere() const;

        /** Finalize the scene.
         *  @details Call both "computeAccelerator" and "computeLightPdfs" 
         *           to finalize the scene. If you update the scene, you should
         *           call this function again.
         */
        void finalize();

        /** Intersection test
         */
        bool intersect(const Ray& ray, Interaction* isect) const;

        /** Add a new shape to the scene.
         *  @param shape The shape added. 
         *  @param bsdf The BSDF for the added shape.
         */
        template <class T>
        void addShape(const T& shape, const BSDF& bsdf) {
            static_assert(std::is_base_of<Shape, T>(), "Template class must be derived from Shape!!");
            _impl->addShape(shape.triangulate(), bsdf);
        }

        void addLight(const std::shared_ptr<Light>& light);

        const std::vector<std::shared_ptr<Light> >& lights() const;

    private:
        class SceneImpl;
        std::unique_ptr<SceneImpl> _impl;
    };

}  // namespace spica

#endif  // _SPICA_SCENE_H_
