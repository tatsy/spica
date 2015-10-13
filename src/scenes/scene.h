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
#else
    #define SPICA_SCENE_DLL
#endif

#include <vector>
#include <memory>
#include <type_traits>

#include "../core/uncopyable.h"
#include "../core/forward_decl.h"

extern void* enabler;

namespace spica {

    /** Scene provides the interface for scene graph.
     */
    class SPICA_SCENE_DLL Scene : private Uncopyable {
    private:
        class SceneImpl;
        std::unique_ptr<SceneImpl> _impl;

    public:
        /** The Scene constructor.
         */
        Scene();

        /** The Scene constructor (move).
         */
        Scene(Scene&& scene);

        /** The Scene destructor.
         */
        ~Scene();

        /** Assignment operator.
         */
        Scene& operator=(Scene&& scene);

        /** Clear current scene.
         */
        void clear();

        /** Set enviroment map to the scene.
         *  @param image The image for enviroment map.
         *  @param camera This is necessary for computing environment sphere.
         */
        void setEnvmap(const Image& image, const Camera& camera);

        /** Compute bounding sphere
         *  @param camera Camera looking at the scene. This is necessary for
         *                computing enviroment map.
         */
        Sphere boundingSphere(const Camera& camera) const;

        /** Return triangle (make new instance in the function)
         */
        Triangle getTriangle(int id) const;
        
        const BSDF& getBsdf(int id) const;

        /** Get direct lighting from specified direction
         */
        Color directLight(const Vector3D& dir) const; 

        /** Sample vertex on the light. This method consumes three random numbers
         */
        LightSample sampleLight(Stack<double>& rstack) const;

        /** Area of light
         */
        double lightArea() const;

        /** Set the type of intersection test accelerator.
         */
        void setAccelType(AccelType accel);

        /** Check specified triangle is light or not.
         *  @details This method is O(log N) implementation
         */
        bool isLightCheck(int id) const;

        /** Compute intersection test accelerator.
         */
        void computeAccelerator();

        /** Finalize the scene.
         *  @details Call both "computeAccelerator" and "computeLightPdfs" 
         *           to finalize the scene. If you update the scene, you should
         *           call this function again.
         */
        void finalize();

        /** Intersection test
         */
        bool intersect(const Ray& ray, Intersection* isect) const;

        bool isTextured(int triID) const;

        int numTriangles() const;

        /** Add a new shape to the scene.
         *  @param shape The shape added. 
         *  @param bsdf The BSDF for the added shape.
         */
        template <class T>
        void addShape(const T& shape, const BSDF& bsdf);

        /** Set area light to the scene
         *  @param shape The shape added for the area light.
         *  @param emission The light emission of the area light
         *  @details If you call setEnvmap before call this function,
         *           this fuction call overwrites the light setting.             
         */
        template <class T>
        void setAreaLight(const T& shape, const Color& emission);
    };

}  // namespace spica

#endif  // _SPICA_SCENE_H_
