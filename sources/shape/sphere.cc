#define SPICA_API_EXPORT
#include "sphere.h"

#include <cmath>

#include "../core/common.h"
#include "../core/interaction.h"

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
        Vector3D dir = ray.dir();
        *isect = SurfaceInteraction(pos, nrm, dir, Point2D());

        return true;
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

                Vector3D v00 = center_ + radius_ * n00;
                Vector3D v01 = center_ + radius_ * n01;
                Vector3D v10 = center_ + radius_ * n10;
                Vector3D v11 = center_ + radius_ * n11;

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
