#ifndef SPICA_RAY_H_
#define SPICA_RAY_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_RAY_EXPORT
        #define SPICA_RAY_DLL __declspec(dllexport)
    #else
        #define SPICA_RAY_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_RAY_DLL
#endif

#include "../utils/vector3.h"

namespace spica {

    class SPICA_RAY_DLL Ray {
    private:
        Vector3 _origin;
        Vector3 _direction;

    public:
        Ray();
        Ray(const Vector3& origin, const Vector3& direction);
        Ray(const Ray& ray);
        ~Ray();

        Ray& operator=(const Ray& ray);

        inline Vector3 origin() const { return _origin; }
        inline Vector3 direction() const { return _direction; }
    };

    class SPICA_RAY_DLL HitPoint {
    private:
        double _distance;
        Vector3 _normal;
        Vector3 _position;

    public:
        HitPoint();
        HitPoint(const HitPoint& hp);
        ~HitPoint();

        HitPoint& operator=(const HitPoint& hp);

        inline double distance() const { return _distance; }
        inline Vector3 normal() const { return _normal; }
        inline Vector3 position() const { return _position; }

        inline void setDistance(double distance) { _distance = distance; }
        inline void setNormal(const Vector3& normal) { _normal = normal; }
        inline void setPosition(const Vector3& position) { _position = position; }
    };

    class SPICA_RAY_DLL Intersection {
    private:
        HitPoint _hitPoint;
        int _objectId;

    public:
        Intersection();
        Intersection(const Intersection& intersection);
        ~Intersection();

        Intersection& operator=(const Intersection& intersection);

        double hittingDistance() const;

        inline int objectId() const { return _objectId; }
        inline const HitPoint& hitPoint() const { return _hitPoint; }

        inline void setHitPoint(const HitPoint& hitpoint) { _hitPoint = hitpoint; }
        inline void setObjectId(int id) { _objectId = id; }
    };

}  // namespace spica

#endif  // SPICA_RAY_H_
