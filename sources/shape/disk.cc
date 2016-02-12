#define SPICA_API_EXPORT
#include "disk.h"

#include "../core/common.h"
#include "../core/interaction.h"

#include "triangle.h"
#include "../renderer/ray.h"
#include "../renderer/renderer_helper.h"

namespace spica {

namespace {

    static const int kSubdivDisk = 64;

}  // anonymous namespace

Disk::Disk() 
    : Shape{ Transform(), ShapeType::Disk } {
}

Disk::Disk(const Point& center, const Normal& normal, double radius,
           const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Disk }
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
    Shape::operator=(disk);
    this->center_ = disk.center_;
    this->normal_ = disk.normal_;
    this->radius_ = disk.radius_;
    return *this;
}

bool Disk::intersect(const Ray& ray, double* tHit, SurfaceInteraction* isect) const {
    double dt = vect::dot(ray.dir(), normal_);
    if (dt > -EPS) return false;

    *tHit = vect::dot(normal_, center_ - ray.org()) / dt;
    Point3D pos = ray.org() + (*tHit) * ray.dir();
    if ((pos - center_).squaredNorm() > radius_ * radius_) return false;

    *isect = SurfaceInteraction(pos, normal_, ray.dir(), Point2D());
    return true;
}

double Disk::area() const {
    return PI * radius_ * radius_;
}

std::vector<Triangle> Disk::triangulate() const {
    Vector3D u, v;
    helper::calcLocalCoords(static_cast<Vector3D>(normal_), &u, &v);

    std::vector<Triangle> retval;
    for (int i = 0; i < kSubdivDisk; i++) {
        int    j  = (i + 1) % kSubdivDisk;
        double t1 = (2.0 * PI * i) / kSubdivDisk;
        double t2 = (2.0 * PI * j) / kSubdivDisk;
        Point p0 = center_ + radius_ * ((u * cos(t1)) + (v * sin(t1)));
        Point p1 = center_ + radius_ * ((u * cos(t2)) + (v * sin(t2)));
        retval.emplace_back(center_, p0, p1, normal_, normal_, normal_);
    }
    return std::move(retval);
}

}  // namepspace spica
