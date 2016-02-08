#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_RAY_H_
#define SPICA_RAY_H_

#include "../math/vector2d.h"
#include "../math/vector3d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../core/spectrum.h"

namespace spica {

    /**
     * Ray class.
     */
    class SPICA_EXPORTS Ray {
    public:
        // Public methods

        /** Constructor. */
        Ray();
        Ray(const Point& origin, const Vector3D& direction);
        Ray(const Ray& ray);
        ~Ray();

        Ray& operator=(const Ray& ray);

        inline Point    origin()    const { return origin_; }
        inline Vector3D direction() const { return direction_; }
        inline Vector3D invdir()    const { return invdir_; }

    private:
        // Private methods
        void calcInvdir();

        // Private fields
        Point    origin_     = { 0.0, 0.0, 0.0 };
        Vector3D direction_  = { 0.0, 0.0, 0.0 };
        Vector3D invdir_     = { INFTY, INFTY, INFTY };

    };

    class SPICA_EXPORTS Hitpoint {
    private:
        double   _distance = INFTY;
        Point    _position = { 0.0, 0.0, 0.0 };
        Normal   _normal   = { 0.0, 0.0, 0.0 };
        Vector2D _texcoord = { INFTY, INFTY };

    public:
        Hitpoint();
        Hitpoint(const Hitpoint& hp);
        ~Hitpoint();

        Hitpoint& operator=(const Hitpoint& hp);

        inline double distance() const { return _distance; }
        inline const Normal& normal() const { return _normal; }
        inline const Point& position() const { return _position; }
        inline const Vector2D& texcoord() const { return _texcoord; }

        inline void setDistance(double distance) { _distance = distance; }
        inline void setNormal(const Normal& normal) { _normal = normal; }
        inline void setPosition(const Point& position) { _position = position; }
        inline void setTexcoord(const Vector2D& texcoord) { _texcoord = texcoord; }
    };

    class SPICA_EXPORTS Intersection {
    private:
        double   _distance = INFTY;
        Point    _pos      = { 0.0, 0.0, 0.0 };
        Normal   _normal   = { 0.0, 0.0, 0.0 };
        Spectrum _color    = { 0.0, 0.0, 0.0 };
        int      _objID    = -1;

    public:
        Intersection();
        Intersection(const Intersection& isect);
        Intersection(int objectID, const Hitpoint& hp, const Spectrum& color);
        ~Intersection();

        Intersection& operator=(const Intersection& intersection);

        inline double   distance() const { return _distance; }
        inline Point    position() const { return _pos; }
        inline Normal   normal()   const { return _normal; }
        inline Spectrum color()    const { return _color; }
        inline int      objectID() const { return _objID; }

        friend class Scene;
    };

}  // namespace spica

#endif  // SPICA_RAY_H_
