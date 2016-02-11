#define SPICA_API_EXPORT
#include "sphere.h"

#include <cmath>

#include "../core/common.h"

#include "trimesh.h"

namespace spica {

    Sphere::Sphere()
        : Shape{ShapeType::Sphere} {
    }

    Sphere::Sphere(const Point& center, double radius)
        : Shape{ ShapeType::Sphere }
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

    bool Sphere::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        const Vector3D VtoC = center_ - ray.origin();
        const double b = VtoC.dot(ray.direction());
        const double D4 = b * b - VtoC.dot(VtoC) + radius_ * radius_;

        if (D4 < 0.0) return false;

        const double sqrtD4 = sqrt(D4);
        const double t1 = b - sqrtD4;
        const double t2 = b + sqrtD4;

        if (t1 < EPS && t2 < EPS) return false;

        if (t1 > EPS) {
            hitpoint->setDistance(t1);
        } else {
            hitpoint->setDistance(t2);
        }

        hitpoint->setPosition(ray.origin() + hitpoint->distance() * ray.direction());
        hitpoint->setNormal(Normal((hitpoint->position() - center_).normalized()));

        return true;
    }

    double Sphere::area() const {
        return 4.0 * PI * radius_ * radius_;
    }

    Trimesh Sphere::triangulate() const {
        static const int nTheta = 64;
        static const int nPhi   = 128;

        std::vector<VertexData> vertices;
        std::vector<Triplet>    faces;
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

                VertexData v00(center_ + radius_ * n00, Spectrum{}, Normal(n00));
                VertexData v01(center_ + radius_ * n01, Spectrum{}, Normal(n01));
                VertexData v10(center_ + radius_ * n10, Spectrum{}, Normal(n10));
                VertexData v11(center_ + radius_ * n11, Spectrum{}, Normal(n11));

                const int idx = static_cast<int>(vertices.size());
                vertices.push_back(v00);
                vertices.push_back(v01);
                vertices.push_back(v10);
                vertices.push_back(v11);

                if (i != nTheta - 1) {
                    faces.emplace_back(idx + 0, idx + 1, idx + 3);
                } 
                
                if (i != 0) {
                    faces.emplace_back(idx + 0, idx + 3, idx + 2);
                }
            }
        }
        return std::move(Trimesh{vertices, faces});
    }

}  // namespace spica
