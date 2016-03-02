#define SPICA_API_EXPORT
#include "sphere.h"

#include <cmath>

#include "../core/common.h"
#include "../core/sampling.h"
#include "../core/interaction.h"
#include "../core/bounds3d.h"
#include "../shape/triangle.h"

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
    const Vector3d dpdu = { -2.0 * PI * pObj.y(), 2.0 * PI * pObj.x(), 0.0 };
    const Vector3d dpdv = -PI * Vector3d(cosPhi * pObj.z(), sinPhi * pObj.z(), -radius_ * sin(theta));

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
    Point3d pObj = Point3d(0.0, 0.0, 0.0) + 
                   radius_ * sampleUniformSphere(rands);
    Normal3d nrm = vect::normalize(objectToWorld_.apply(Normal3d(pObj)));
    return Interaction{ pObj, nrm };
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
    Vector3d pObj = radius_ * nObj;

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
    static const int nTheta = 64;
    static const int nPhi   = 128;

    std::vector<Triangle> tris;
    for (int i = 0; i < nTheta; i++) {
        for (int j = 0; j < nPhi; j++) {
            double theta0 = PI * i / nTheta;
            double theta1 = PI * (i + 1) / nTheta; 
            double phi0 = 2.0 * PI * j / nPhi;
            double phi1 = 2.0 * PI * (j + 1) / nPhi;

            double st0 = sin(theta0);
            double st1 = sin(theta1);
            double ct0 = cos(theta0);
            double ct1 = cos(theta1);
            double sp0 = sin(phi0);
            double sp1 = sin(phi1);
            double cp0 = cos(phi0);
            double cp1 = cos(phi1);

            Vector3d n00(cp0 * st0, sp0 * st0, ct0);
            Vector3d n01(cp0 * st1, sp0 * st1, ct1);
            Vector3d n10(cp1 * st0, sp1 * st0, ct0);
            Vector3d n11(cp1 * st1, sp1 * st1, ct1);

            Point3d v00 = center_ + radius_ * n00;
            Point3d v01 = center_ + radius_ * n01;
            Point3d v10 = center_ + radius_ * n10;
            Point3d v11 = center_ + radius_ * n11;

            if (i != nTheta - 1) {
                tris.emplace_back(v00, v01, v11,
                                  Normal3d(n00), Normal3d(n01), Normal3d(n11));
            }

            if (i != 0) {
                tris.emplace_back(v00, v11, v10,
                                  Normal3d(n00), Normal3d(n11), Normal3d(n10));
            }
        }
    }
    return std::move(tris);
}

}  // namespace spica
