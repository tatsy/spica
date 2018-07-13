#define SPICA_API_EXPORT
#include "triangle.h"

#include "core/bounds3d.h"
#include "core/interaction.h"

namespace spica {

Triangle::Triangle()
    : Shape{ Transform(), ShapeType::Triangle }
    , points_{}
    , normals_{}
    , uvs_{}
    , faceNormal_{} {
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ objectToWorld.apply(p0),
               objectToWorld.apply(p1),
               objectToWorld.apply(p2) }
    , normals_{}
    , uvs_{}
    , faceNormal_{} {
    // Compute face normals
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];
    faceNormal_ = Normal3d(Vector3d::cross(e1, e2).normalized());
    normals_ = { faceNormal_, faceNormal_, faceNormal_ };
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Normal3d& n0, const Normal3d& n1, const Normal3d& n2,
                   const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ objectToWorld.apply(p0), 
               objectToWorld.apply(p1),
               objectToWorld.apply(p2) }
    , normals_{ Normal3d(objectToWorld.apply(Vector3d(n0))).normalized(),
                Normal3d(objectToWorld.apply(Vector3d(n1))).normalized(),
                Normal3d(objectToWorld.apply(Vector3d(n2))).normalized() }
    , uvs_{} {
    // Compute normals
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];
    faceNormal_ = Normal3d(vect::cross(e1, e2));
    if (faceNormal_.norm() < EPS) {
        faceNormal_ = (normals_[0] + normals_[1] + normals_[2]) / 3.0;
    }
    faceNormal_ = vect::normalize(faceNormal_);
}

Triangle::Triangle(const Point3d& p0, const Point3d& p1, const Point3d& p2,
                   const Normal3d& n0, const Normal3d& n1, const Normal3d& n2,
                   const Point2d& uv0, const Point2d& uv1, const Point2d& uv2,
                   const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Triangle }
    , points_{ objectToWorld.apply(p0),
               objectToWorld.apply(p1),
               objectToWorld.apply(p2) }
    , normals_{ Normal3d(objectToWorld.apply(Vector3d(n0))).normalized(),
                Normal3d(objectToWorld.apply(Vector3d(n1))).normalized(),
                Normal3d(objectToWorld.apply(Vector3d(n2))).normalized() }
    , uvs_{ uv0, uv1, uv2 } {
    // Compute normals
    const Vector3d e1 = points_[1] - points_[0];
    const Vector3d e2 = points_[2] - points_[0];
    faceNormal_ = Normal3d(vect::cross(e1, e2));
    if (faceNormal_.norm() < EPS) {
        faceNormal_ = (normals_[0] + normals_[1] + normals_[2]) / 3.0;
    }
    faceNormal_ = vect::normalize(faceNormal_);
}

Triangle::Triangle(const Triangle& t)
    : Triangle{} {
    this->operator=(t);
}

Triangle::~Triangle() {
}

Triangle& Triangle::operator=(const Triangle& t) {
    Shape::operator=(t);
    this->points_     = t.points_;
    this->normals_    = t.normals_;
    this->uvs_        = t.uvs_;
    this->faceNormal_ = t.faceNormal_;
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
    Point2d  uv  = (1.0 - u - v) * uvs_[0] + u * uvs_[1] + v * uvs_[2];

    const Point2d duv01 = uvs_[1] - uvs_[0];
    const Point2d duv02 = uvs_[2] - uvs_[0];
    const double detUV = duv01.x() * duv02.y() - duv01.y() * duv02.x();
    
    Vector3d dpdu, dpdv;
    if (detUV == 0.0) {
        vect::coordinateSystem(Vector3d(faceNormal_), &dpdu, &dpdv);
    } else {
        const double invdet = 1.0 / detUV;
        const double invM[2][2] = { {  duv02.y() * invdet, -duv01.y() * invdet },
                                    { -duv02.x() * invdet,  duv01.x() * invdet } };
        const Vector3d dp01 = points_[1]  - points_[0];
        const Vector3d dp02 = points_[2]  - points_[0];
        dpdu = invM[0][0] * dp01 + invM[0][1] * dp02;
        dpdv = invM[1][0] * dp01 + invM[1][1] * dp02;
    }
    *isect = SurfaceInteraction(pos, uv, -ray.dir(), dpdu, dpdv, Normal3d(), Normal3d(), this);

    // Compute shading geometry
    Normal3d ns = vect::normalize((1.0 - u - v) * normals_[0] + u * normals_[1] + v * normals_[2]);
    if (std::abs(vect::dot(ns, faceNormal_)) < 1.0 - EPS) {
        Normal3d ss = vect::normalize(vect::cross(ns, faceNormal_));
        Normal3d ts = vect::normalize(vect::cross(ns, ss));

        Normal3d dndu, dndv;
        const Normal3d dn01 = normals_[1] - normals_[0];
        const Normal3d dn02 = normals_[2] - normals_[0];
        const double invdet = 1.0 / detUV;
        const double invM[2][2] = { {  duv02.y() * invdet, -duv01.y() * invdet },
                                    { -duv02.x() * invdet,  duv01.x() * invdet } };
        dndu = invM[0][0] * dn01 + invM[0][1] * dn02;
        dndv = invM[1][0] * dn01 + invM[1][1] * dn02;
        isect->setShadingGeometry(Vector3d(ss), Vector3d(ts), dndu, dndv);
    }
    return true;
}

bool Triangle::intersect(const Ray& ray) const {
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

    const double tHit = Vector3d::dot(e2, qVec) * invdet;
    return (tHit > EPS && tHit <= ray.maxDist());
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
    Normal3d nrm = (1.0 - u0 - u1) * normals_[0] + u0 * normals_[1] +
                   u1 * normals_[2];
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

} // namespace spica
