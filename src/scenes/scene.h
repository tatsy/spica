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
#elif
    #define SPICA_SCENE_DLL
#endif

#include <vector>
#include <memory>
#include <type_traits>

#include "../core/uncopyable.h"
#include "../core/forward_decl.h"

extern void* enabler;

namespace spica {

    class Camera;

    /** Scene provides the interface for scene graph.
     */
    class SPICA_SCENE_DLL Scene : private Uncopyable {
    private:
        class SceneImpl;
        std::unique_ptr<SceneImpl> _impl;

    public:
        Scene();
        Scene(Scene&& scene);
        ~Scene();

        Scene& operator=(Scene&& scene);

        void clear();

        /** Set enviroment map to the scene
         *  @param image the image for enviroment map
         *  @param camera it's necessary for computing environment sphere
         */
        void setEnvmap(const Image& image, const Camera& camera);

        //! Compute bounding sphere
        Sphere boundingSphere(const Camera& camera) const;

        //! Return triangle (make new instance in the function)
        Triangle getTriangle(int id) const;
        
        const BSDF& getBsdf(int id) const;

        Color getReflectance(const Intersection& isect) const;

        // Get direct lightinf from specified direction
        Color directLight(const Vector3D& dir) const; 

        //! Sample vertex on the light. This method consumes three random numbers
        LightSample sampleLight(Stack<double>& rstack) const;

        //! area of light
        double lightArea() const;

        void setAccelType(AccelType accel);

        // Check specified triangle is light or not
        // This method is O(log N) implementation
        bool isLightCheck(int id) const;

        void computeAccelerator();

        // Call both "computeAccelerator" and "computeLightPdfs" 
        // to finalize the scene. If you update the scene, you should
        // call this function again.
        void finalize();

        bool intersect(const Ray& ray, Intersection* isect) const;

        inline bool isTextured(int triID) const;

        inline int numTriangles() const;

        void addShape(const BBox& shape, const BSDF& bsdf);
        void addShape(const Disk& shape, const BSDF& bsdf);
        void addShape(const Quad& shape, const BSDF& bsdf);
        void addShape(const Sphere& shape, const BSDF& bsdf);
        void addShape(const Triangle& shape, const BSDF& bsdf);
        void addShape(const Trimesh& shape, const BSDF& bsdf);
        
        //! Set area light to the scene
        void setAreaLight(const BBox& shape, const Color& emission);
        void setAreaLight(const Disk& shape, const Color& emission);
        void setAreaLight(const Quad& shape, const Color& emission);
        void setAreaLight(const Sphere& shape, const Color& emission);
        void setAreaLight(const Triangle& shape, const Color& emission);
        void setAreaLight(const Trimesh& shape, const Color& emission);
    };

}  // namespace spica

#endif  // _SPICA_SCENE_H_
