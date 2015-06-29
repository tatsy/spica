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

#include "../utils/common.h"
#include "../utils/vector3.h"
#include "../utils/uncopyable.h"
#include "../geometry/geometry.h"

#include "envmap.h"
#include "ray.h"
#include "material.h"

namespace spica {
    
    class SPICA_SCENE_DLL Scene : public Uncopyable {
    private:
        unsigned int _nPrimitives;
        unsigned int _arraySize;
        int _lightID;
        const Primitive** _primitives;
        Material* _materials;
        Envmap _envmap;

    public:
        Scene();
        ~Scene();

        template <class Ty>
        void add(const Ty& primitive, const Material& material, bool isLight = false) {
            static_assert(std::is_base_of<Primitive, Ty>::value, "Type inherits Primitive can only be added to the scene.");
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

        inline const Envmap& envmap() const { return _envmap; }
        inline void setEnvmap(const Envmap& envmap) { _envmap = envmap; }

    private:
        Scene(const Scene& scene);
        Scene& operator=(const Scene& scene);

        void init();
        void release();

        void checkArraySize();
    };
}

#endif  // _SPICA_SCENE_H_
