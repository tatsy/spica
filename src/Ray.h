#ifndef RAINY_RAY_H_
#define RAINY_RAY_H_

#include "Vector3.h"

namespace rainy {

    class Ray {
    private:
        Vector3 _origin;
        Vector3 _direction;

    public:
        Ray();
        Ray(const Vector3& origin, const Vector3& direction);
        Ray(const Ray& ray);
        ~Ray();

        Ray& operator=(const Ray& ray);
    };

    class HitPoint {
    private:
        double _distance;
        Vector3 _normal;
        Vector3 _position;

    public:
        HitPoint();
        HitPoint(const HitPoint& hp);
        ~HitPoint();

        HitPoint& operator=(const HitPoint& hp);
    };

    class Intersection {
    private:
        HitPoint _hitPoint;
        int _objectId;

    public:
        Intersection();
        Intersection(const Intersection& intersection);
        ~Intersection();

        Intersection& operator=(const Intersection& intersection);
    };

}  // namespace rainy

#endif  // RAINY_RAY_H_
