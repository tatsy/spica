#define SPICA_API_EXPORT
#include "disk.h"

#include "../core/common.h"
#include "../renderer/ray.h"
#include "../renderer/renderer_helper.h"

#include "trimesh.h"

namespace spica {

    namespace {

        static const int kSubdivDisk = 64;

    }  // anonymous namespace

    Disk::Disk() 
        : IShape{ShapeType::Disk} {
    }

    Disk::Disk(const Point& center, const Normal& normal, double radius)
        : IShape{ ShapeType::Disk }
        , center_{ center }
        , normal_{ normal }
        , radius_{ radius } {
    }

    Disk::Disk(const Disk& disk)
        : Disk{} {
        this->operator=(disk);
    }

    Disk::~Disk() {
    }

    Disk& Disk::operator=(const Disk& disk) {
        this->center_ = disk.center_;
        this->normal_ = disk.normal_;
        this->radius_ = disk.radius_;
        return *this;
    }

    bool Disk::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        double dt = vect::dot(ray.direction(), normal_);
        if (dt > -EPS) return false;

        double tHit = vect::dot(normal_, center_ - ray.origin()) / dt;
        hitpoint->setDistance(tHit);
        hitpoint->setPosition(ray.origin() + tHit * ray.direction());
        hitpoint->setNormal(normal_);
        return (hitpoint->position() - center_).norm() <= radius_;
    }

    double Disk::area() const {
        return PI * radius_ * radius_;
    }

    Trimesh Disk::triangulate() const {
        Vector3D u, v;
        helper::calcLocalCoords(static_cast<Vector3D>(normal_), &u, &v);

        std::vector<Triangle> retval;
        for (int i = 0; i < kSubdivDisk; i++) {
            int    j  = (i + 1) % kSubdivDisk;
            double t1 = (2.0 * PI * i) / kSubdivDisk;
            double t2 = (2.0 * PI * j) / kSubdivDisk;
            Point p0 = center_ + radius_ * ((u * cos(t1)) + (v * sin(t1)));
            Point p1 = center_ + radius_ * ((u * cos(t2)) + (v * sin(t2)));
            retval.emplace_back(center_, p0, p1);
        }
        return std::move(retval);
    }

}  // namepspace spica
