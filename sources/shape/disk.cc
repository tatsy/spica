#define SPICA_API_EXPORT
#include "disk.h"

#include "../core/common.h"
#include "../core/interaction.h"

#include "triangle.h"
#include "../core/ray.h"
#include "../integrator/renderer_helper.h"

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
    Vector3D p2c = pos - center_;
    const double r = p2c.norm();
    if (r > radius_) return false;

    Vector3D uVec, vVec;
    vect::coordinateSystem(Vector3D(normal_), &uVec, &vVec);
    const double theta = atan2(vect::dot(vVec, p2c), vect::dot(uVec, p2c)); 
    const double u = r / radius_;
    const double v = (theta + PI) / (2.0 * PI);

    const double cosTheta = cos(theta);
    const double sinTheta = sin(theta);
    const Vector3D dpdu = radius_ * (uVec * cosTheta + vVec * sinTheta);
    const Vector3D dpdv = 2.0 * PI * r * (-uVec * sinTheta + vVec * cosTheta);

    *isect = SurfaceInteraction(pos, Point2D(u, v), -ray.dir(), dpdu, dpdv, Normal(0.0, 0.0, 0.0), Normal(0.0, 0.0, 0.0), this);
    return true;
}

Bound3d Disk::objectBound() const {
    Bound3d b;
    Vector3D u, v;
    vect::coordinateSystem(Vector3D(normal_), &u, &v);
    b.merge(center_ + radius_ * u);
    b.merge(center_ + radius_ * v);
    return b;
}

double Disk::area() const {
    return PI * radius_ * radius_;
}

std::vector<Triangle> Disk::triangulate() const {
    Vector3D u, v;
    vect::coordinateSystem(Vector3D(normal_), &u, &v);

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
