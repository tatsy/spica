#define SPICA_DISK_EXPORT
#include "disk.h"

#include "../core/common.h"
#include "../renderer/ray.h"
#include "../renderer/renderer_helper.h"

namespace spica {

    namespace {

        const int kSubdivDisk = 64;

    }  // anonymous namespace

    Disk::Disk() 
        : IShape{ShapeType::Disk} {
    }

    Disk::Disk(const Vector3D& center, const Vector3D& normal, double radius)
        : IShape{ShapeType::Disk}
        , _center{center}
        , _normal{normal}
        , _radius{radius} {
    }

    Disk::Disk(const Disk& disk)
        : Disk{} {
        this->operator=(disk);
    }

    Disk::~Disk() {
    }

    Disk& Disk::operator=(const Disk& disk) {
        this->_center = disk._center;
        this->_normal = disk._normal;
        this->_radius = disk._radius;
        return *this;
    }

    bool Disk::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        double dt = Vector3D::dot(ray.direction(), _normal);
        if (dt > -EPS) return false;

        double tHit = Vector3D::dot(_normal, _center - ray.origin()) / dt;
        hitpoint->setDistance(tHit);
        hitpoint->setPosition(ray.origin() + tHit * ray.direction());
        hitpoint->setNormal(_normal);
        return (hitpoint->position() - _center).norm() <= _radius;            
    }

    double Disk::area() const {
        return PI * _radius * _radius;
    }

    std::vector<Triangle> Disk::triangulate() const {
        Vector3D u, v;
        helper::calcLocalCoords(this->_normal, &u, &v);

        std::vector<Triangle> retval;
        for (int i = 0; i < kSubdivDisk; i++) {
            int    j  = (i + 1) % kSubdivDisk;
            double t1 = (2.0 * PI * i) / kSubdivDisk;
            double t2 = (2.0 * PI * j) / kSubdivDisk;
            Vector3D p0 = _center + _radius * ((u * cos(t1)) + (v * sin(t1)));
            Vector3D p1 = _center + _radius * ((u * cos(t2)) + (v * sin(t2)));
            retval.emplace_back(_center, p0, p1);
        }
        return std::move(retval);
    }

}  // namepspace spica
