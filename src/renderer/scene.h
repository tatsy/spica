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
#include "../utils/stack.h"
#include "../utils/uncopyable.h"
#include "../geometry/geometry.h"
#include "../light/lighting.h"

#include "bsdf.h"
#include "ray.h"

namespace spica {

    class Camera;

    class SPICA_SCENE_DLL Scene {
    private:
        std::vector<Triangle> _triangles;
        std::vector<Color> _emittance;

        std::vector<unsigned int> _bsdfIds;
        std::vector<unsigned int> _lightIds;

        std::vector<BSDF> _bsdfs;
        std::shared_ptr<AccelBase> _accel;

        Lighting _lighting;
        AccelType _accelType;

    public:
        Scene();
        Scene(const Scene& scene);
        ~Scene();

        Scene& operator=(const Scene& scene);

        template <class T>
        void addShape(const T& shape, const BSDF& bsdf);
        
        //! Set area light to the scene
        template <class T>
        void setLight(const T& shape, const Color& emittance);

        //! Set environment map to the scene
        // @param[in] filename: name of the environment map image file
        void setEnvmap(const std::string& filename, const Camera& camera);

        //! Compute bounding sphere
        Sphere boundingSphere(const Camera& camera) const;

        const Triangle& getTriangle(int id) const;
        const BSDF& getBsdf(int id) const;

        // Get direct lightinf from specified direction
        Color directLight(const Vector3D& dir) const; 

        //! Sample vertex on the light. This method consumes three random numbers
        LightSample sampleLight(Stack<double>& rstack) const;

        //! area of light
        double lightArea() const;

        void clear();

        void setAccelType(AccelType accel);

        // Check specified triangle is light or not
        // This method is O(log N) implementation
        bool isLightCheck(int id) const;

        void computeAccelerator();

        // Call both "computeAccelerator" and "computeLightPdfs" 
        // to finalize the scene. If you update the scene, you should
        // call this function again.
        void finalize();

        bool intersect(const Ray& ray, Intersection& isect) const;

        inline size_t numTriangles() const { return _triangles.size(); }

        friend class Lighting;
    };
}

// Implementation for the template functions
#include "scene_detail.h"

#endif  // _SPICA_SCENE_H_
