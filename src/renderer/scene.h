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
#include "../geometry/geometry.h"

#include "ray.h"
#include "material.h"

namespace spica {
    
    class SPICA_SCENE_DLL Scene {
    private:
        unsigned int _nPrimitives;
        unsigned int _arraySize;
        int _lightID;
        const Primitive** _primitives;
        Material* _materials;
        Color _bgColor;

    public:
        Scene();
        ~Scene();

        template <class Ty>
        void add(const Ty& primitive, const Material& material, bool isLight = false) {
            //msg_assert(std::is_same<Ty, Primitive>::value, "Input primitive is not instance of Primitive");

            if (isLight) _lightID = _nPrimitives;
            _primitives[_nPrimitives] = new Ty(primitive);
            _materials[_nPrimitives] = material;
            _nPrimitives++;
            checkArraySize();
        }

        const Primitive* get(int id) const;
        const Material& getMaterial(int id) const;

        void clear();

        bool intersect(const Ray& ray, Intersection& isect) const;

        inline int lightID() const { return _lightID; }
        inline int numObjects() const { return _nPrimitives; }
        inline const Color& bgColor() const { return _bgColor; }

        inline void setBgColor(const Color& bgColor) { _bgColor = bgColor; }


    private:
        Scene(const Scene& scene);
        Scene& operator=(const Scene& scene);

        void init();
        void release();

        void checkArraySize();
    };
}

#endif  // SPICA_SCENE_H_
