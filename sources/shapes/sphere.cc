#define SPICA_API_EXPORT
#include "sphere.h"

#include <cmath>

#include "core/common.h"
#include "core/bounds3d.h"
#include "core/triangle.h"
#include "render/interaction.h"
#include "render/sampling.h"

namespace spica {

Sphere::Sphere()
    : Shape{ Transform{}, ShapeType::Sphere } {
}

Sphere::Sphere(const Point3d& center, double radius, const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Sphere }
    , center_{ center }
    , radius_{ radius } {
}

Sphere::Sphere(const Sphere& sphere)
    : Sphere{} {
    this->operator=(sphere);
}

Sphere::~Sphere() {
}

Sphere& Sphere::operator=(const Sphere& sphere) {
    this->radius_ = sphere.radius_;
    this->center_ = sphere.center_;
    return *this;
}

bool Sphere::intersect(const Ray& ray, double* tHit,
                        SurfaceInteraction* isect) const {
    // Compute intersection
    const Vector3d VtoC = center_ - ray.org();
    const double b = VtoC.dot(ray.dir());
    const double D4 = b * b - VtoC.dot(VtoC) + radius_ * radius_;

    if (D4 < 0.0) return false;

    const double sqrtD4 = sqrt(D4);
    const double t1 = b - sqrtD4;
    const double t2 = b + sqrtD4;

    if (t1 < EPS && t2 < EPS) return false;

    if (t1 > EPS) {
        *tHit = t1;
    } else {
        *tHit = t2;
    }
    if (*tHit > ray.maxDist()) return false;

    Point3d  pWorld = ray.org() + (*tHit) * ray.dir();
    Point3d  pObj   = Point3d(pWorld - center_);
    Normal3d nrm    = Normal3d(pObj).normalized();

    // Compute differential geometries
    const double phi   = vect::sphericalPhi(nrm);
    const double theta = vect::sphericalTheta(nrm);
    const double cosPhi = cos(phi);
    const double sinPhi = sin(phi);
    const double u = phi / (2.0 * PI);
    const double v = theta  / PI;
    Vector3d dpdu = Vector3d{ -2.0 * PI * pObj.y(), 2.0 * PI * pObj.x(), 0.0 };
    Vector3d dpdv = -PI * Vector3d(cosPhi * pObj.z(), sinPhi * pObj.z(),
                                   -radius_ * sin(theta));

    // Prevent dpdu becomes zero when the ray intersect on either north or south pole.
    if (dpdu.norm() < EPS) {
        double sz = pObj.z() > 0.0 ? 1.0 : -1.0;
        if (dpdv.cross(Vector3d(sz, 0.0, 0.0)).z() > EPS) {
            dpdu = dpdv.cross(Vector3d(sz, 0.0, 0.0));
        } else {
            dpdu = dpdv.cross(Vector3d(0.0, sz, 0.0));
        }
    }


    Vector3d d2pdudu = - (2.0 * PI) * (2.0 * PI) * Vector3d(pObj.x(), pObj.y(), 0.0);
    Vector3d d2pdudv = PI * pObj.z() * (2.0 * PI) * Vector3d(sinPhi, -cosPhi, 0.0);
    Vector3d d2pdvdv = -PI * PI * pObj;

    // Fundamental forms
    const double E = Vector3d::dot(dpdu, dpdu);
    const double F = Vector3d::dot(dpdu, dpdv);
    const double G = Vector3d::dot(dpdv, dpdv);
    const Vector3d N = Vector3d::cross(dpdu, dpdv).normalized();
    const double e = Vector3d::dot(N, d2pdudu);
    const double f = Vector3d::dot(N, d2pdudv);
    const double g = Vector3d::dot(N, d2pdvdv);

    double invEGF2 = 1.0 / (E * G - F * F);
    Normal3d dndu = Normal3d((F * f - G * e) * invEGF2 * dpdu + (F * e - E * f) * invEGF2 * dpdv);
    Normal3d dndv = Normal3d((F * g - G * f) * invEGF2 * dpdu + (F * f - E * g) * invEGF2 * dpdv);

    *isect = SurfaceInteraction(pWorld, Point2d(u, v), -ray.dir(), dpdu, dpdv, dndu, dndv, this);
    return true;
}

bool Sphere::intersect(const Ray& ray) const {
    // Compute intersection
    const Vector3d v2c = center_ - ray.org();
    const double b = v2c.dot(ray.dir());
    const double D4 = b * b - v2c.squaredNorm() + radius_ * radius_;

    if (D4 < 0.0) return false;

    const double sqrtD4 = sqrt(D4);
    const double t1 = b - sqrtD4;
    const double t2 = b + sqrtD4;

    if (t1 < EPS && t2 < EPS) return false;

    double tHit;
    if (t1 > EPS) {
        tHit = t1;
    } else {
        tHit = t2;
    }

    return tHit <= ray.maxDist();
}

Interaction Sphere::sample(const Point2d& rands) const {
    Vector3d pLocal = sampleUniformSphere(rands);
    Point3d pObj = center_ + radius_ * pLocal;
    Normal3d pNorm = Normal3d(pLocal).normalized();
    return Interaction{ pObj, pNorm };
}

Interaction Sphere::sample(const Interaction& isect,
                           const Point2d& rands) const {
    // TODO: For sphere (because it's a closed geometry),
    //       the case where the intersection is inside the sphere
    //       should be considered.

    double dist2 = (isect.pos() - center_).squaredNorm();
    double sinThetaMax2 = radius_ * radius_ / dist2;
    double cosThetaMax  = sqrt(std::max(0.0, 1.0 - sinThetaMax2));
    double cosTheta = (1.0 - rands[0]) + rands[0] * cosThetaMax;
    double sinTheta = sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
    double phi = rands[1] * 2.0 * PI;

    double dc = sqrt(dist2);
    double ds = dc * cosTheta - sqrt(std::max(0.0, radius_ * radius_ - dc * dc * sinTheta * sinTheta));
    double cosAlpha = (dc * dc + radius_ * radius_ - ds * ds) / (2.0 * dc * radius_);
    double sinAlpha = sqrt(std::max(0.0, 1.0 - cosAlpha * cosAlpha));

    Vector3d sw = vect::normalize(isect.pos() - center_);
    Vector3d su, sv;
    vect::coordinateSystem(sw, &su, & sv);
    Vector3d nObj = su * cos(phi) * cosAlpha + sv * sin(phi) * cosAlpha + sw * sinAlpha;
    Vector3d pObj = center_ + radius_ * nObj;

    return Interaction{ Point3d(pObj), Normal3d(nObj) };
}

double Sphere::pdf(const Interaction& pObj, const Vector3d& wi) const {
    // TODO: For sphere (because it's a closed geometry),
    //       the case where the intersection is inside the sphere
    //       should be considered.

    double dist2 = (pObj.pos() - center_).squaredNorm();
    double sinTheta2 = radius_ * radius_ / dist2;
    double cosTheta = sqrt(std::max(0.0, 1.0 - sinTheta2));
    return 1.0 / (2.0 * PI * (1.0 - cosTheta));
}

Bounds3d Sphere::objectBound() const {
    const Point3d posMin = center_ - Vector3d(radius_, radius_, radius_);
    const Point3d posMax = center_ + Vector3d(radius_, radius_, radius_);
    return { posMin, posMax };
}

double Sphere::area() const {
    return 4.0 * PI * radius_ * radius_;
}

std::vector<Triangle> Sphere::triangulate() const {
    const double phi = (1.0 + std::sqrt(5.0)) * 0.5;
    std::vector<Point3d> points;
    points.emplace_back(0.0, -1.0, -phi);
    points.emplace_back(0.0,  1.0, -phi);
    points.emplace_back(0.0, -1.0,  phi);
    points.emplace_back(0.0,  1.0,  phi);
    points.emplace_back(-phi, 0.0, -1.0);
    points.emplace_back(-phi, 0.0,  1.0);
    points.emplace_back( phi, 0.0, -1.0);
    points.emplace_back( phi, 0.0,  1.0);
    points.emplace_back(-1.0, -phi, 0.0);
    points.emplace_back( 1.0, -phi, 0.0);
    points.emplace_back(-1.0,  phi, 0.0);
    points.emplace_back( 1.0,  phi, 0.0);
    for (auto& p : points) {
        p = vect::normalize(p);
    }

    std::vector<Triangle> tris;
    tris.emplace_back(points[0], points[1], points[6]);
    tris.emplace_back(points[1], points[0], points[4]);
    tris.emplace_back(points[2], points[3], points[5]);
    tris.emplace_back(points[3], points[2], points[7]);
    tris.emplace_back(points[4], points[5], points[10]);

    tris.emplace_back(points[5], points[4], points[8]);
    tris.emplace_back(points[6], points[7], points[9]);
    tris.emplace_back(points[7], points[6], points[11]);
    tris.emplace_back(points[8], points[9], points[2]);
    tris.emplace_back(points[9], points[8], points[0]);

    tris.emplace_back(points[10], points[11], points[1]);
    tris.emplace_back(points[11], points[10], points[3]);
    tris.emplace_back(points[0], points[6], points[9]);
    tris.emplace_back(points[0], points[8], points[4]);
    tris.emplace_back(points[1], points[4], points[10]);

    tris.emplace_back(points[1], points[11], points[6]);
    tris.emplace_back(points[2], points[5], points[8]);
    tris.emplace_back(points[2], points[9], points[7]);
    tris.emplace_back(points[3], points[7], points[11]);
    tris.emplace_back(points[3], points[10], points[5]);

    const int maxDepth = 4;
    for (int i = 0; i < maxDepth; i++) {
        std::vector<Triangle> temp;
        for (const auto& t : tris) {
            Point3d p0 = t[0];
            Point3d p1 = t[1];
            Point3d p2 = t[2];
            Point3d p01 = vect::normalize(0.5 * (p0 + p1));
            Point3d p12 = vect::normalize(0.5 * (p1 + p2));
            Point3d p20 = vect::normalize(0.5 * (p2 + p0));
            temp.emplace_back(p0, p01, p20);
            temp.emplace_back(p1, p12, p01);
            temp.emplace_back(p2, p20, p12);
            temp.emplace_back(p01, p12, p20);
        }
        tris = std::move(temp);
    }

    std::vector<Triangle> ret;
    for (const auto& t : tris) {
        Point3d p0 = t[0] * radius_ + center_;
        Point3d p1 = t[1] * radius_ + center_;
        Point3d p2 = t[2] * radius_ + center_;
        Normal3d n0(t[0]);
        Normal3d n1(t[1]);
        Normal3d n2(t[2]);
        ret.emplace_back(p0, p1, p2, n0, n1, n2);
    }

    return std::move(ret);
}

}  // namespace spica
