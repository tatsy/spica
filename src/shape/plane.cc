#define SPICA_API_EXPORT
#include "plane.h"

#include "../core/common.h"
#include "../renderer/ray.h"

namespace spica {

    Plane::Plane()
        : IShape{ShapeType::Plane} {
    }

    Plane::Plane(double distance, const Vector3D& normal)
        : IShape{ShapeType::Plane}
        , _distance{distance}
        , _normal{normal} {
    }

    Plane::Plane(const Plane& plane) 
        : IShape{ShapeType::Plane}
        , _distance{plane._distance}
        , _normal{plane._normal} {
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
        if (dt > -EPS) {
            return false;
        }

        double dist = - _distance / dt;
        hitpoint->setDistance(dist);
        hitpoint->setPosition(ray.origin() + dist * ray.direction());
        hitpoint->setNormal(_normal);
        return true;
    }

    double Plane::area() const {
        Assertion(false, "Area of Plane is infinite!!");
        return INFTY;
    }

    std::vector<Triangle> Plane::triangulate() const {
        Assertion(false, "Plane has infinite area, "
                         "which cannot be triangulated!!");
    }

}  // namespace spica
