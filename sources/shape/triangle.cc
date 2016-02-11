#define SPICA_API_EXPORT
#include "triangle.h"

#include "../core/common.h"
#include "../renderer/ray.h"

#include "trimesh.h"

namespace spica {

    Triangle::Triangle()
        : Shape{ ShapeType::Triangle }
        , points_{} {
    }

    Triangle::Triangle(const Point& p0, const Point& p1, const Point& p2)
        : Shape{ ShapeType::Triangle }
        , points_{ p0, p1, p2 } {
    }

    Triangle::Triangle(const Triangle& tri)
        : Triangle{} {
        this->operator=(tri);
    }

    Triangle::~Triangle() {
    }

    Triangle& Triangle::operator=(const Triangle& tri) {
        this->points_ = tri.points_;
        return *this;
    }

    Point Triangle::get(int id) const {
        Assertion(0 <= id && id <= 2, "ID must be between 0 and 2");
        return points_[id];
    }

    Point Triangle::operator[](int id) const {
        Assertion(0 <= id && id <= 2, "ID must be between 0 and 2");
        return points_[id];
    }

    Normal Triangle::normal() const {
        const Vector3D e1 = points_[1] - points_[0];
        const Vector3D e2 = points_[2] - points_[0];
        return Normal(Vector3D::cross(e1, e2).normalized());
    }

    Point Triangle::gravity() const {
        return (points_[0] + points_[1] + points_[2]) / 3.0;
    }

    bool Triangle::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        const Vector3D e1 = points_[1] - points_[0];
        const Vector3D e2 = points_[2] - points_[0];
        Vector3D pVec = Vector3D::cross(ray.direction(), e2);
        double det = Vector3D::dot(e1, pVec);
        if (det > -EPS && det <EPS) return false;
        double invdet = 1.0 / det;
        
        const Vector3D tVec = ray.origin() - points_[0];
        double u = Vector3D::dot(tVec, pVec) * invdet;
        if (u < 0.0 || u > 1.0) return false;

        const Vector3D qVec = Vector3D::cross(tVec, e1);
        double v = Vector3D::dot(ray.direction(), qVec) * invdet;
        if (v < 0.0 || u + v > 1.0) return false;

        double t = Vector3D::dot(e2, qVec) * invdet;
        if (t > EPS) {
            hitpoint->setDistance(t);
            hitpoint->setPosition(ray.origin() + t * ray.direction());
            hitpoint->setNormal(this->normal());
            hitpoint->setTexcoord(Vector2D(u, v));
            return true;
        }

        return false;
    }

    double Triangle::area() const {
        const Vector3D e1 = points_[1] - points_[0];
        const Vector3D e2 = points_[2] - points_[0];
        return 0.5 * Vector3D::cross(e1, e2).norm();
    }

    Trimesh Triangle::triangulate() const {
        std::vector<Triangle> tris{1, *this};
        return std::move(Trimesh{tris});
    }

}  // namespace spica
