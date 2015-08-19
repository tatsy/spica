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
        std::vector<Color> _emittance;

        std::vector<unsigned int> _bsdfIds;
        std::vector<unsigned int> _lightIds;
        std::vector<double> _lightPdfs;
        double _totalLightArea;

        std::vector<BSDF> _bsdfs;
        std::shared_ptr<AccelBase> _accel;
        AccelType _accelType;

        Envmap _envmap;

    public:
        Scene();
        ~Scene();

        template <class Ty>
        void add(const Ty& geom, const BSDF& bsdf, const Color& emittance = Color(0.0, 0.0, 0.0), bool isLight = false) {
            static_assert(std::is_base_of<IGeometry, Ty>::value, "Type inherits IGeometry can only be added to the scene !!");

            std::vector<Triangle> newTriangles = geom.triangulate();

            // If new object is a light, store triangle indices
            if (isLight) {
                for (int i = 0; i < newTriangles.size(); i++) {
                    _lightIds.push_back(_triangles.size() + i);
                }
            }

            // Both normal geoms and light geoms are set to triangles
            _triangles.insert(_triangles.end(), newTriangles.begin(), newTriangles.end());

            // Compute and store emittance
            for (int i = 0; i < newTriangles.size(); i++) {
                _emittance.push_back(emittance);
            }
            
            // Update BSDF ids
            const int newBsdfId = static_cast<int>(_bsdfs.size());
            const int numTriangles = static_cast<int>(_bsdfIds.size());
            _bsdfIds.resize(_bsdfIds.size() + newTriangles.size());
            std::fill(_bsdfIds.begin() + numTriangles, _bsdfIds.end(), newBsdfId);
            _bsdfs.push_back(bsdf);
        }

        const Triangle& getTriangle(int id) const;
        const BSDF& getBsdf(int id) const;
        const Color& getEmittance(int id) const;

        int sampleLight(double rand1) const;

        void clear();

        void setAccelType(AccelType accel);

        void computeAccelerator();
        void computeLightPdfs();

        // Call both "computeAccelerator" and "computeLightPdfs" to finalize the scene.
        // If you update the scene, you should call this function again.
        void finalize();

        bool intersect(const Ray& ray, Intersection& isect) const;

        inline size_t numTriangles() const { return _triangles.size(); }

        inline const Envmap& envmap() const { return _envmap; }
        inline void setEnvmap(const Envmap& envmap) { _envmap = envmap; }
        inline double totalLightArea() const { return _totalLightArea; }
    };
}

#endif  // _SPICA_SCENE_H_
