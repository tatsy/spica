#define SPICA_DISK_EXPORT
#include "disk.h"

#include "../utils/common.h"

namespace spica {

    Disk::Disk()
        : _center()
        , _normal()
        , _radius(0.0)
    {
    }

    Disk::Disk(const Vector3& center, const Vector3& normal, double radius)
        : _center(center)
        , _normal(normal)
        , _radius(radius)
    {
    }

    Disk::Disk(const Disk& disk)
        : _center()
        , _normal()
        , _radius()
    {
        operator=(disk);
    }

    Disk::~Disk() 
    {
    }

    Disk& Disk::operator=(const Disk& disk) {
        this->_center = disk._center;
        this->_normal = disk._normal;
        this->_radius = disk._radius;
        return *this;
    }

    bool Disk::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        double tHit = (_center - ray.origin()).dot(ray.direction());
        hitpoint->setDistance(tHit);
        hitpoint->setPosition(ray.origin() + tHit * ray.direction());
        hitpoint->setNormal(ray.direction().dot(_normal) < 0.0 ? _normal : -_normal);
        return (hitpoint->position() - _center).norm() < _radius;            
    }

    double Disk::area() const {
        return PI * _radius * _radius;
    }

}  // namepspace spica

