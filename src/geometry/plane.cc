#define SPICA_PLANE_EXPORT
#include "plane.h"

#include "../utils/common.h"

namespace spica {
    Plane::Plane()
        : _distance(0.0)
        , _normal()
    {
    }

    Plane::Plane(double distance, const Vector3& normal)
        : _distance(distance)
        , _normal(normal)
    {
    }

    Plane::Plane(const Plane& plane) 
        : _distance(plane._distance)
        , _normal(plane._normal)
    {
    }

    Plane::~Plane() {
    }

    Plane& Plane::operator=(const Plane& plane) {
        this->_distance = plane._distance;
        this->_normal   = plane._normal;
        return *this;
    }

    bool Plane::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        double dt = ray.direction().dot(_normal);
        if (dt > EPS) {
            return false;
        }

        double dist = - _distance / dt;
        hitpoint->setDistance(dist);
        hitpoint->setPosition(ray.origin() + dist * ray.direction());
        hitpoint->setNormal(_normal);
        return true;
    }

    double Plane::area() const {
        msg_assert(false, "Area of Plane geometry is INFTY. It can make unexpected results.");
        return INFTY;
    }
}
