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

namespace spica {

    /** Scene provides the interface for scene graph.
     */
    class SPICA_EXPORTS Scene : private Uncopyable {
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

        /** Set environment map to the scene.
         *  @param image The image for enviroment map.
         */
        void setEnvmap(const Image& image);

        /** Get environment map of the scene.
         *  @return If lighting does not use environment map, then returns the black image.
         */
        Image getEnvmap() const;

        /** Compute bounding sphere
         */
        Sphere boundingSphere() const;

        /** Return triangle (make new instance in the function)
         */
        Triangle getTriangle(int id) const;
        
        const BSDF& getBsdf(int id) const;

        /** Get direct lighting from specified direction
         */
        Color directLight(const Vector3D& dir) const; 

        /** Get lighting of global enviroment.
         *  @details This method is usually used to sample light when the ray does not intersect anything.
         */
        Color globalLight(const Vector3D& dir) const;

        /** Sample vertex on the light. This method consumes three random numbers.
         */
        LightSample sampleLight(const Vector3D& v, Stack<double>& rands) const;

        /** Sample photon.
         */
        Photon samplePhoton(Stack<double>& rands) const;

        /** Area of light.
         */
        double lightArea() const;

        /** Get lighting type.
         */
        LightType lightType() const;

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
