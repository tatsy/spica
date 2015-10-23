#define SPICA_API_EXPORT
#include "triangle.h"

#include "../core/common.h"
#include "../renderer/ray.h"

#include "trimesh.h"

namespace spica {

    Triangle::Triangle()
        : IShape{ShapeType::Triangle}
        , _points{} {
    }

    Triangle::Triangle(const Vector3D& p0, const Vector3D& p1, const Vector3D& p2)
        : IShape{ShapeType::Triangle}
        , _points{p0, p1, p2} {
    }

    Triangle::Triangle(const Triangle& tri)
        : Triangle{} {
        this->operator=(tri);
    }

    Triangle::~Triangle() {
    }

    Triangle& Triangle::operator=(const Triangle& tri) {
        this->_points = tri._points;
        return *this;
    }

    Vector3D Triangle::get(int id) const {
        Assertion(0 <= id && id <= 2, "ID must be between 0 and 2");
        return _points[id];
    }

    Vector3D Triangle::operator[](int id) const {
        Assertion(0 <= id && id <= 2, "ID must be between 0 and 2");
        return _points[id];
    }

    Vector3D Triangle::normal() const {
        const Vector3D e1 = _points[1] - _points[0];
        const Vector3D e2 = _points[2] - _points[0];
        return Vector3D::cross(e1, e2).normalized();
    }

    Vector3D Triangle::gravity() const {
        return (_points[0] + _points[1] + _points[2]) / 3.0;
    }

    bool Triangle::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        Vector3D e1 = _points[1] - _points[0];
        Vector3D e2 = _points[2] - _points[0];
        Vector3D pVec = Vector3D::cross(ray.direction(), e2);
        double det = Vector3D::dot(e1, pVec);
        if (det > -EPS && det <EPS) return false;
        double invdet = 1.0 / det;
        
        Vector3D tVec = ray.origin() - _points[0];
        double u = Vector3D::dot(tVec, pVec) * invdet;
        if (u < 0.0 || u > 1.0) return false;

        Vector3D qVec = Vector3D::cross(tVec, e1);
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
        const Vector3D e1 = _points[1] - _points[0];
        const Vector3D e2 = _points[2] - _points[0];
        return 0.5 * Vector3D::cross(e1, e2).norm();
    }

    Trimesh Triangle::triangulate() const {
        std::vector<Triangle> tris{1, *this};
        return std::move(Trimesh{tris});
    }

}  // namespace spica
