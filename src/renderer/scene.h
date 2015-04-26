#ifndef SPICA_SCENE_H_
#define SPICA_SCENE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SCENE_EXPORT
        #define SPICA_SCENE_DLL __declspec(dllexport)
    #else
        #define SPICA_SCENE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_SCENE_DLL
#endif

#include "../utils/common.h"
#include "../utils/vector3.h"
#include "../geometry/plane.h"
#include "../geometry/sphere.h"
#include "ray.h"

namespace spica {
    
    class SPICA_SCENE_DLL Scene {
    private:
        unsigned int _nPrimitives;
        unsigned int _arraySize;
        int _lightID;
        Primitive** _primitives;
        Color _bgColor;

    public:
        Scene();
        ~Scene();

        void addPlane(const Plane& plane, bool isLight = false);
        void addSphere(const Sphere& sphere, bool isLight = false);

        const Primitive* getObjectPtr(int id) const;

        void release();

        bool intersect(const Ray& ray, Intersection& intersection) const;

        inline int lightID() const { return _lightID; }
        inline const Color& bgColor() const { return _bgColor; }

        inline void setBgColor(const Color& bgColor) { _bgColor = bgColor; }


    private:
        Scene(const Scene& scene);
        Scene& operator=(const Scene& scene);

        void checkArraySize();
    };
}

#endif  // SPICA_SCENE_H_
