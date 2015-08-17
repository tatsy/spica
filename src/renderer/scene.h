#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_H_
#define _SPICA_SCENE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SCENE_EXPORT
        #define SPICA_SCENE_DLL __declspec(dllexport)
    #else
        #define SPICA_SCENE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_SCENE_DLL
#endif

#include <vector>
#include <memory>

#include "../utils/common.h"
#include "../utils/vector3d.h"
#include "../utils/uncopyable.h"
#include "../geometry/geometry.h"

#include "bsdf.h"
#include "envmap.h"
#include "ray.h"

namespace spica {
    
    class SPICA_SCENE_DLL Scene : private Uncopyable {
    private:
        std::vector<Triangle> _triangles;
        std::vector<Triangle> _lights;
        std::vector<unsigned int> _bsdfIds;
        std::vector<BSDF> _bsdfs;
        std::shared_ptr<AccelBase> _accel;
        Envmap _envmap;

    public:
        Scene();
        ~Scene();

        template <class Ty>
        void add(const Ty& geom, const BSDF& bsdf, bool isLight = false) {
            static_assert(std::is_base_of<IGeometry, Ty>::value, "Type inherits IGeometry can only be added to the scene !!");

            std::vector<Triangle> newTriangles = geom.triangulate();
            if (isLight) {
                // If geometry is a light, set it to lights
                _lights.insert(_lights.end(), newTriangles.begin(), newTriangles.end());
            }

            // Both normal geoms and light geoms are set to triangles
            _triangles.insert(_triangles.end(), newTriangles.begin(), newTriangles.end());
            
            // Update BSDF ids
            const int newBsdfId = static_cast<int>(_bsdfs.size());
            const int numTriangles = static_cast<int>(_bsdfIds.size());
            _bsdfIds.resize(_bsdfIds.size() + newTriangles.size());
            std::fill(_bsdfIds.begin() + numTriangles, _bsdfIds.end(), newBsdfId);
            _bsdfs.push_back(bsdf);
        }

        const Triangle& getTriangle(int id) const;
        const BSDF& getBsdf(int id) const;

        void clear();
        void setAccelerator(AccelType accel = QBVH_ACCEL);

        bool intersect(const Ray& ray, Intersection& isect) const;

        inline size_t numTriangles() const { return _triangles.size(); }

        inline const Envmap& envmap() const { return _envmap; }
        inline void setEnvmap(const Envmap& envmap) { _envmap = envmap; }
    };
}

#endif  // _SPICA_SCENE_H_
