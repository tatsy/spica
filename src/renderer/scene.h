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

#include "../utils/common.h"
#include "../utils/vector3d.h"
#include "../utils/uncopyable.h"
#include "../geometry/geometry.h"

#include "envmap.h"
#include "ray.h"
#include "material.h"

namespace spica {
    
    class SPICA_SCENE_DLL Scene : public Uncopyable {
    private:
        unsigned int _nIGeometrys;
        unsigned int _arraySize;
        int _lightID;
        const IGeometry** _primitives;
        Material* _materials;
        Envmap _envmap;

    public:
        Scene();
        ~Scene();

        template <class Ty>
        void add(const Ty& primitive, const Material& material, bool isLight = false) {
            static_assert(std::is_base_of<IGeometry, Ty>::value, "Type inherits IGeometry can only be added to the scene.");
            if (isLight) _lightID = _nIGeometrys;
            _primitives[_nIGeometrys] = new Ty(primitive);
            _materials[_nIGeometrys] = material;
            _nIGeometrys++;
            checkArraySize();
        }

        const IGeometry* get(int id) const;
        const Material& getMaterial(int id) const;

        void clear();

        bool intersect(const Ray& ray, Intersection& isect) const;

        inline int lightID() const { return _lightID; }
        inline int numObjects() const { return _nIGeometrys; }

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
