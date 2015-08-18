#define SPICA_SPHERE_EXPORT
#include "sphere.h"

#include <cmath>

#include "../utils/common.h"
#include "triangle.h"

namespace spica {

    Sphere::Sphere()
        : _radius(0.0)
        , _center()
    {
    }

    Sphere::Sphere(double radius, const Vector3D& center)
        :  _radius(radius)
        , _center(center)
    {
    }

    Sphere::Sphere(const Sphere& sphere)
        : _radius()
        , _center()
    {
        operator=(sphere);
    }

    Sphere::~Sphere()
    {
    }

    Sphere& Sphere::operator=(const Sphere& sphere) {
        this->_radius = sphere._radius;
        this->_center = sphere._center;
        return *this;
    }

    bool Sphere::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        const Vector3D VtoC = _center - ray.origin();
        const double b = VtoC.dot(ray.direction());
        const double D4 = b * b - VtoC.dot(VtoC) + _radius * _radius;

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
        hitpoint->setNormal((hitpoint->position() - _center).normalized());

        return true;
    }

    double Sphere::area() const {
        return 4.0 * PI * _radius * _radius;
    }

    std::vector<Triangle> Sphere::triangulate() const {
        static const int nTheta = 128;
        static const int nPhi   = 256;

        std::vector<Triangle> retval;
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

                Vector3D v00 = _center + _radius * Vector3D(cp0 * st0, sp0 * st0, ct0);
                Vector3D v01 = _center + _radius * Vector3D(cp0 * st1, sp0 * st1, ct1);
                Vector3D v10 = _center + _radius * Vector3D(cp1 * st0, sp1 * st0, ct0);
                Vector3D v11 = _center + _radius * Vector3D(cp1 * st1, sp1 * st1, ct1);

                if (i != nTheta - 1) {
                    retval.emplace_back(v00, v01, v11);
                } 
                
                if (i != 0) {
                    retval.emplace_back(v00, v10, v01);
                } 
            }
        }
        return std::move(retval);
    }

}  // namespace spica
