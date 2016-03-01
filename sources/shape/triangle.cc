#define SPICA_API_EXPORT
#include "triangle.h"

#include "../core/bounds3d.h"
#include "../core/interaction.h"

namespace spica {

Triangle::Triangle()
    : Shape{ Transform(), ShapeType::Triangle }
    , points_{}
    , normals_{}
    , uvs_{} {
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ objectToWorld.apply(p0),
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
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ objectToWorld.apply(p0), 
               objectToWorld.apply(p1),
               objectToWorld.apply(p2) }
    , normals_{ objectToWorld.apply(n0),
                objectToWorld.apply(n1), 
                objectToWorld.apply(n2) }
    , uvs_{} {
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Normal3d& n0, const Normal3d& n1, const Normal3d& n2,
                   const Point2d& uv0, const Point2d& uv1, const Point2d& uv2,
                   const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ p0, p1, p2 }
    , normals_{ n0, n1, n2 }
    , uvs_{ uv0, uv1, uv2 } {
}

Triangle::Triangle(const Triangle& t)
    : Triangle{} {
    this->operator=(t);
}

Triangle::~Triangle() {
}

Triangle& Triangle::operator=(const Triangle& t) {
    Shape::operator=(t);
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
                         SurfaceInteraction* isect) const {
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

    *tHit = Vector3d::dot(e2, qVec) * invdet;
    if (*tHit <= EPS || *tHit > ray.maxDist()) return false;

    Point3d  pos = ray.org() + (*tHit) * ray.dir();
    Normal3d nrm = (1.0 - u - v) * normals_[0] + u * normals_[1] + v * normals_[2];
    Point2d  uv  = (1.0 - u - v) * uvs_[0] + u * uvs_[1] + v * uvs_[2];

    const Point2d duv01 = uvs_[1] - uvs_[0];
    const Point2d duv02 = uvs_[2] - uvs_[0];
    const double detUV = duv01.x() * duv02.y() - duv01.y() * duv02.x();
    
    Vector3d dpdu, dpdv;
    Normal3d dndu, dndv;
    if (detUV == 0.0) {
        vect::coordinateSystem(vect::normalize(vect::cross(points_[1] - points_[0], points_[2] - points_[0])), &dpdu, &dpdv);
    } else {
        const double invdet = 1.0 / detUV;
        const double invM[2][2] = { {  duv02.y() * invdet, -duv01.y() * invdet },
                                    { -duv02.x() * invdet,  duv01.x() * invdet } };
        const Vector3d dp01 = points_[1]  - points_[0];
        const Vector3d dp02 = points_[2]  - points_[0];
        const Normal3d dn01 = normals_[1] - normals_[0];
        const Normal3d dn02 = normals_[2] - normals_[0];
        dpdu = invM[0][0] * dp01 + invM[0][1] * dp02;
        dpdv = invM[1][0] * dp01 + invM[1][1] * dp02;
        dndu = invM[0][0] * dn01 + invM[0][1] * dn02;
        dndv = invM[1][0] * dn01 + invM[1][1] * dn02;
    }
    *isect = SurfaceInteraction(pos, uv, -ray.dir(), dpdu, dpdv, dndu, dndv, this);
    return true;
}

Interaction Triangle::sample(const Point2d& rands) const {
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

    Point3d  pos = points_[0] + u0 * e1 + u1 * e2;
    Normal3d nrm = (1.0 - u0 - u1) * normals_[0] + u0 * normals_[1] + u1 * normals_[2];
    return Interaction{ pos, nrm };    
}

Bounds3d Triangle::worldBound() const {
    Point3d posMin = Point3d::minimum(points_[0], Point3d::minimum(points_[1], points_[2]));
    Point3d posMax = Point3d::maximum(points_[0], Point3d::maximum(points_[1], points_[2]));
    return Bounds3d{ posMin, posMax };
}

Bounds3d Triangle::objectBound() const {
    Point3d posMin = Point3d::minimum(points_[0], Point3d::minimum(points_[1], points_[2]));
    Point3d posMax = Point3d::maximum(points_[0], Point3d::maximum(points_[1], points_[2]));
    return Bounds3d{ worldToObject_.apply(posMin),
                     worldToObject_.apply(posMax) };
}

double Triangle::area() const {
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];
    return 0.5 * Vector3d::cross(e1, e2).norm();
}

std::vector<Triangle> Triangle::triangulate() const {
    std::vector<Triangle> tris(1, *this);
    return std::move(tris);
}

Point3d Triangle::gravity() const {
    return (points_[0] + points_[1] + points_[2]) / 3.0;
}

}  // namespace spica
