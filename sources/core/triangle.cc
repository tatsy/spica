#define SPICA_API_EXPORT
#include "triangle.h"

#include "../core/bounds3d.h"

namespace spica {

Triangle::Triangle()
    : points_{}
    , normals_{}
    , uvs_{} {
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Transform& objectToWorld)
    : points_{ objectToWorld.apply(p0),
               objectToWorld.apply(p1),
               objectToWorld.apply(p2) }
    , normals_{}
    , uvs_{} {
    // Compute normals
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];
    const Normal3d nn = Normal3d(Vector3d::cross(e1, e2).normalized());
    normals_ = { nn, nn, nn };
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Normal3d& n0, const Normal3d& n1, const Normal3d& n2,
                   const Transform& objectToWorld)
    : points_{ objectToWorld.apply(p0), 
               objectToWorld.apply(p1),
               objectToWorld.apply(p2) }
    , normals_{ Normal3d(objectToWorld.apply(Vector3d(n0))).normalized(),
                Normal3d(objectToWorld.apply(Vector3d(n1))).normalized(), 
                Normal3d(objectToWorld.apply(Vector3d(n2))).normalized() }
    , uvs_{} {
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Normal3d& n0, const Normal3d& n1, const Normal3d& n2,
                   const Point2d& uv0, const Point2d& uv1, const Point2d& uv2,
                   const Transform& objectToWorld)
    : points_{ objectToWorld.apply(p0),
               objectToWorld.apply(p1),
               objectToWorld.apply(p2) }
    , normals_{ Normal3d(objectToWorld.apply(Vector3d(n0))).normalized(),
                Normal3d(objectToWorld.apply(Vector3d(n1))).normalized(), 
                Normal3d(objectToWorld.apply(Vector3d(n2))).normalized() }
    , uvs_{ uv0, uv1, uv2 } {
}

Triangle::Triangle(const Triangle& t)
    : Triangle{} {
    this->operator=(t);
}

Triangle::~Triangle() {
}

Triangle& Triangle::operator=(const Triangle& t) {
    this->points_  = t.points_;
    this->normals_ = t.normals_;
    this->uvs_     = t.uvs_;
    return *this;
}

const Point3d& Triangle::operator[](int i) const {
    Assertion(i >= 0 && i <= 2, "Index out of bounds!!");
    return points_[i];
}

bool Triangle::intersect(const Ray& ray, double* tHit,
                         Point2d *uv) const {
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];
    Vector3d pVec = Vector3d::cross(ray.dir(), e2);

    double det = Vector3d::dot(e1, pVec);
    if (det > -EPS && det <EPS) return false;

    double invdet = 1.0 / det;        
    const Vector3d tVec = ray.org() - points_[0];
    double u = Vector3d::dot(tVec, pVec) * invdet;
    if (u < 0.0 || u > 1.0) return false;

    const Vector3d qVec = Vector3d::cross(tVec, e1);
    double v = Vector3d::dot(ray.dir(), qVec) * invdet;
    if (v < 0.0 || u + v > 1.0) return false;

    double tTemp = Vector3d::dot(e2, qVec) * invdet;
    if (tTemp <= EPS || tTemp > ray.maxDist()) return false;

    *tHit = tTemp;
    *uv  = (1.0 - u - v) * uvs_[0] + u * uvs_[1] + v * uvs_[2];
    
    return true;
}

Point2d Triangle::sample(const Point2d& rands, Point3d *pos, Normal3d *nrm) const {
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];

    double u0, u1;
    if (rands[0] + rands[1] >= 1.0) {
        u0 = 1.0 - rands[0];
        u1 = 1.0 - rands[1];
    } else {
        u0 = rands[0];
        u1 = rands[1];
    }

    *pos = points_[0] + u0 * e1 + u1 * e2;
    *nrm = (1.0 - u0 - u1) * normals_[0] + u0 * normals_[1] +
                   u1 * normals_[2];

    return Point2d(u0, u1);
}

double Triangle::area() const {
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];
    return 0.5 * Vector3d::cross(e1, e2).norm();
}

const Normal3d& Triangle::normal(int i) const {
    Assertion(i >= 0 && i <= 2, "Index out of bounds: %d specified.", i);
    return normals_[i];
}

const Point2d& Triangle::uv(int i) const {
    Assertion(i >= 0 && i <= 2, "Index out of bounds: %d specified.", i);
    return uvs_[i];
}

Point3d Triangle::gravity() const {
    return (points_[0] + points_[1] + points_[2]) / 3.0;
}

}  // namespace spica
