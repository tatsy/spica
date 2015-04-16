#ifndef RAINY_SCENE_H_
#define RAINY_SCENE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef RAINY_SCENE_EXPORT
        #define RAINY_SCENE_DLL __declspec(dllexport)
    #else
        #define RAINY_SCENE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define RAINY_SCENE_DLL
#endif

#include "common.h"
#include "Vector3.h"
#include "Plane.h"
#include "Sphere.h"
#include "Ray.h"

namespace spica {
    
    class RAINY_SCENE_DLL Scene {
    private:
        unsigned int _nPrimitives;
        unsigned int _arraySize;
        Primitive** _primitives;

    public:
        Scene();
        ~Scene();

        void addPlane(const Plane& plane);
        void addSphere(const Sphere& sphere);

        const Primitive* getObjectPtr(int id) const;

        void release();

        bool intersect(const Ray& ray, Intersection& intersection) const;

    private:
        Scene(const Scene& scene);
        Scene& operator=(const Scene& scene);

        void checkArraySize();
    };
}

#endif  // RAINY_SCENE_H_
