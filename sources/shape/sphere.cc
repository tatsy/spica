#define SPICA_API_EXPORT
#include "sphere.h"

#include <cmath>

#include "../core/common.h"
#include "../core/interaction.h"
#include "../core/bound3d.h"
#include "../shape/triangle.h"

namespace spica {

Sphere::Sphere()
    : Shape{ Transform{}, ShapeType::Sphere } {
}

Sphere::Sphere(const Point& center, double radius, const Transform& objectToWorld)
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
    const Vector3D VtoC = center_ - ray.org();
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

    Point  pos = ray.org() + (*tHit) * ray.dir();
    Normal nrm = Normal(pos - center_).normalized();

    // Compute differential geometries
    const double phi   = vect::sphericalPhi(nrm);
    const double theta = vect::sphericalTheta(nrm);
    const double cosPhi = cos(phi);
    const double sinPhi = sin(phi);
    const double u = phi / (2.0 * PI);
    const double v = (theta + PI / 2.0) / PI;
    const Vector3D dpdu = { -2.0 * PI * pos.y(), 2.0 * PI * pos.x(), 0.0 };
    const Vector3D dpdv = PI * Vector3D(cosPhi * pos.z(), sinPhi * pos.z(), -radius_ * sin(theta));

    Vector3D d2pdudu = - (2.0 * PI) * (2.0 * PI) * Vector3D(pos.x(), pos.y(), 0.0);
    Vector3D d2pdudv = PI * pos.z() * (2.0 * PI) * Vector3D(-sinPhi, cosPhi, 0.0);
    Vector3D d2pdvdv = -PI * PI * pos;

    // Fundamental forms
    const double E = Vector3D::dot(dpdu, dpdu);
    const double F = Vector3D::dot(dpdu, dpdv);
    const double G = Vector3D::dot(dpdv, dpdv);
    const Vector3D N = Vector3D::cross(dpdu, dpdv).normalized();
    const double e = Vector3D::dot(N, d2pdudu);
    const double f = Vector3D::dot(N, d2pdudv);
    const double g = Vector3D::dot(N, d2pdvdv);

    double invEGF2 = 1.0 / (E * G - F * F);
    Normal3D dndu = Normal3D((F * f - G * e) * invEGF2 * dpdu + (F * e - E * f) * invEGF2 * dpdv);
    Normal3D dndv = Normal3D((F * g - G * f) * invEGF2 * dpdu + (F * f - E * g) * invEGF2 * dpdv);

    Vector3D dir = ray.dir();
    *isect = SurfaceInteraction(pos, Point2D(u, v), -ray.dir(), dpdu, dpdv, dndu, dndv, this);

    return true;
}

Interaction Sphere::sample(const Interaction& isect,
                           const Point2D& rands) const {
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

    Vector3D sw = vect::normalize(isect.pos() - center_);
    Vector3D su, sv;
    vect::coordinateSystem(sw, &su, & sv);
    Vector3D nObj = su * cos(phi) * cosAlpha + sv * sin(phi) * cosAlpha + sw * sinAlpha;
    Vector3D pObj = radius_ * nObj;

    return Interaction{ Point(pObj), Normal(nObj) };


}

double Sphere::pdf(const Interaction& pObj, const Vector3D& wi) const {
    // TODO: For sphere (because it's a closed geometry),
    //       the case where the intersection is inside the sphere
    //       should be considered.

    double dist2 = (pObj.pos() - center_).squaredNorm();
    double sinTheta2 = radius_ * radius_ / dist2;
    double cosTheta = sqrt(std::max(0.0, 1.0 - sinTheta2));
    return 1.0 / (2.0 * PI * (1.0 - cosTheta));
}

Bound3d Sphere::objectBound() const {
    const Point3D posMin = center_ - Vector3D(radius_, radius_, radius_);
    const Point3D posMax = center_ + Vector3D(radius_, radius_, radius_);
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

            Vector3D n00(cp0 * st0, sp0 * st0, ct0);
            Vector3D n01(cp0 * st1, sp0 * st1, ct1);
            Vector3D n10(cp1 * st0, sp1 * st0, ct0);
            Vector3D n11(cp1 * st1, sp1 * st1, ct1);

            Point3D v00 = center_ + radius_ * n00;
            Point3D v01 = center_ + radius_ * n01;
            Point3D v10 = center_ + radius_ * n10;
            Point3D v11 = center_ + radius_ * n11;

            if (i != nTheta - 1) {
                tris.emplace_back(v00, v01, v11, Normal(n00), Normal(n01), Normal(n11));
            }

            if (i != 0) {
                tris.emplace_back(v00, v11, v10, Normal(n00), Normal(n11), Normal(n10));
            }
        }
    }
    return std::move(tris);
}

}  // namespace spica
