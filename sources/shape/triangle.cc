#define SPICA_API_EXPORT
#include "triangle.h"

#include "../core/bound3d.h"
#include "../core/interaction.h"

namespace spica {

Triangle::Triangle()
    : Shape{ Transform(), ShapeType::Triangle }
    , points_{}
    , normals_{}
    , uvs_{} {
}

Triangle::Triangle(const Point3D& p0, const Point3D& p1, const Point3D& p2,
                   const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ p0, p1, p2 }
    , normals_{}
    , uvs_{} {
    // Compute normals
    const Vector3D e1 = points_[1] - points_[0];
    const Vector3D e2 = points_[2] - points_[0];
    const Normal nn = Normal(Vector3D::cross(e1, e2).normalized());
    normals_ = { nn, nn, nn };
}

Triangle::Triangle(const Point3D& p0, const Point3D& p1, const Point3D& p2,
                   const Normal3D& n0, const Normal3D& n1, const Normal3D& n2,
                   const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ p0, p1, p2 }
    , normals_{ n0, n1, n2 }
    , uvs_{} {
}

Triangle::Triangle(const Point3D& p0, const Point3D& p1, const Point3D& p2,
                   const Normal3D& n0, const Normal3D& n1, const Normal3D& n2,
                   const Point2D& uv0, const Point2D& uv1, const Point2D& uv2,
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

const Point3D& Triangle::operator[](int i) const {
    Assertion(i >= 0 && i <= 2, "Index out of bounds!!");
    return points_[i];
}

bool Triangle::intersect(const Ray& ray, double* tHit,
                         SurfaceInteraction* isect) const {
    const Vector3D e1 = points_[1] - points_[0];
    const Vector3D e2 = points_[2] - points_[0];
    Vector3D pVec = Vector3D::cross(ray.dir(), e2);

    double det = Vector3D::dot(e1, pVec);
    if (det > -EPS && det <EPS) return false;

    double invdet = 1.0 / det;        
    const Vector3D tVec = ray.org() - points_[0];
    double u = Vector3D::dot(tVec, pVec) * invdet;
    if (u < 0.0 || u > 1.0) return false;

    const Vector3D qVec = Vector3D::cross(tVec, e1);
    double v = Vector3D::dot(ray.dir(), qVec) * invdet;
    if (v < 0.0 || u + v > 1.0) return false;

    *tHit = Vector3D::dot(e2, qVec) * invdet;
    if (*tHit <= EPS) return false;

    Point3D pos = ray.org() + (*tHit) * ray.dir();
    Normal  nrm = (1.0 - u - v) * normals_[0] + u * normals_[1] + v * normals_[2];
    Point2D uv  = (1.0 - u - v) * uvs_[0] + u * uvs_[1] + v * uvs_[2];

    const Point2D duv01 = uvs_[1] - uvs_[0];
    const Point2D duv02 = uvs_[2] - uvs_[0];
    const double detUV = duv01.x() * duv02.y() - duv01.y() * duv02.x();
    
    Vector3D dpdu, dpdv;
    Normal   dndu, dndv;
    if (detUV == 0.0) {
        vect::coordinateSystem(vect::normalize(vect::cross(points_[1] - points_[0], points_[2] - points_[0])), &dpdu, &dpdv);
    } else {
        const double invdet = 1.0 / detUV;
        const double invM[2][2] = { {  duv02.y() * invdet, -duv01.y() * invdet },
                                    { -duv02.x() * invdet,  duv01.x() * invdet } };
        const Vector3D dp01 = points_[1]  - points_[0];
        const Vector3D dp02 = points_[2]  - points_[0];
        const Normal   dn01 = normals_[1] - normals_[0];
        const Normal   dn02 = normals_[2] - normals_[0];
        dpdu = invM[0][0] * dp01 + invM[0][1] * dp02;
        dpdv = invM[1][0] * dp01 + invM[1][1] * dp02;
        dndu = invM[0][0] * dn01 + invM[0][1] * dn02;
        dndv = invM[1][0] * dn01 + invM[1][1] * dn02;
    }

    *isect = SurfaceInteraction(pos, uv, -ray.dir(), dpdu, dpdv, dndu, dndv, this);
    return true;
}

Interaction Triangle::sample(const Interaction& isect,
                             const Point2D& rands) const {
    const Vector3D e1 = points_[1] - points_[0];
    const Vector3D e2 = points_[2] - points_[0];

    double u0, u1;
    if (rands[0] + rands[1] >= 1.0) {
        u0 = 1.0 - rands[0];
        u1 = 1.0 - rands[1];
    } else {
        u0 = rands[0];
        u1 = rands[1];
    }

    Point3D pos = points_[0] + u0 * e1 + u1 * e2;
    Normal  nrm = (1.0 - u0 - u1) * normals_[0] + u0 * normals_[1] + u1 * normals_[2];
    return Interaction{ pos, nrm };
}

Bound3d Triangle::objectBound() const {
    Point3D posMin = Point3D::minimum(points_[0], Point3D::minimum(points_[1], points_[2]));
    Point3D posMax = Point3D::maximum(points_[0], Point3D::maximum(points_[1], points_[2]));
    return Bound3d{ posMin, posMax };
}

double Triangle::area() const {
    const Vector3D e1 = points_[1] - points_[0];
    const Vector3D e2 = points_[2] - points_[0];
    return 0.5 * Vector3D::cross(e1, e2).norm();
}

std::vector<Triangle> Triangle::triangulate() const {
    std::vector<Triangle> tris(1, *this);
    return std::move(tris);
}

Point3D Triangle::gravity() const {
    return (points_[0] + points_[1] + points_[2]) / 3.0;
}

}  // namespace spica
