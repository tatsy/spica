#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_RAY_H_
#define SPICA_RAY_H_

#include "../math/vector2d.h"
#include "../math/vector3d.h"
#include "../core/color.h"

namespace spica {

    class SPICA_EXPORTS Ray {
    private:
        Vector3D _origin     = { 0.0, 0.0, 0.0 };
        Vector3D _direction  = { 0.0, 0.0, 0.0 };
        Vector3D _invdir     = { INFTY, INFTY, INFTY };

    public:
        Ray();
        Ray(const Vector3D& origin, const Vector3D& direction);
        Ray(const Ray& ray);
        ~Ray();

        Ray& operator=(const Ray& ray);

        inline Vector3D origin()    const { return _origin; }
        inline Vector3D direction() const { return _direction; }
        inline Vector3D invdir()    const { return _invdir; }

    private:
        void calcInvdir();
    };

    class SPICA_EXPORTS Hitpoint {
    private:
        double   _distance = INFTY;
        Vector3D _position = { 0.0, 0.0, 0.0 };
        Vector3D _normal   = { 0.0, 0.0, 0.0 };
        Vector2D _texcoord = { INFTY, INFTY };

    public:
        Hitpoint();
        Hitpoint(const Hitpoint& hp);
        ~Hitpoint();

        Hitpoint& operator=(const Hitpoint& hp);

        inline double distance() const { return _distance; }
        inline const Vector3D& normal() const { return _normal; }
        inline const Vector3D& position() const { return _position; }
        inline const Vector2D& texcoord() const { return _texcoord; }

        inline void setDistance(double distance) { _distance = distance; }
        inline void setNormal(const Vector3D& normal) { _normal = normal; }
        inline void setPosition(const Vector3D& position) { _position = position; }
        inline void setTexcoord(const Vector2D& texcoord) { _texcoord = texcoord; }
    };

    class SPICA_EXPORTS Intersection {
    private:
        double   _distance = INFTY;
        Vector3D _pos      = { 0.0, 0.0, 0.0 };
        Vector3D _normal   = { 0.0, 0.0, 0.0 };
        Color    _color    = { 0.0, 0.0, 0.0 };
        int      _objID    = -1;

    public:
        Intersection();
        Intersection(const Intersection& isect);
        Intersection(int objectID, const Hitpoint& hp, const Color& color);
        ~Intersection();

        Intersection& operator=(const Intersection& intersection);

        inline double   distance() const { return _distance; }
        inline Vector3D position() const { return _pos; }
        inline Vector3D normal()   const { return _normal; }
        inline Color    color()    const { return _color; }
        inline int      objectID() const { return _objID; }

        friend class Scene;
    };

}  // namespace spica

#endif  // SPICA_RAY_H_
