#define SPICA_TRIANGLE_EXPORT
#include "triangle.h"

#include "../utils/common.h"

namespace spica {

    Triangle::Triangle()
        : _p0()
        , _p1()
        , _p2()
    {
    }

    Triangle::Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2)
        : _p0(p0)
        , _p1(p1)
        , _p2(p2)
    {
    }

    Triangle::Triangle(const Triangle& tri)
        : _p0(tri._p0)
        , _p1(tri._p1)
        , _p2(tri._p2)
    {
    }

    Triangle::~Triangle()
    {
    }

    Triangle& Triangle::operator=(const Triangle& tri) {
        this->_p0 = tri._p0;
        this->_p1 = tri._p1;
        this->_p2 = tri._p2;
        return *this;
    }

    Vector3 Triangle::p(int id) const {
        msg_assert(0 <= id && id <= 2, "ID must be between 0 and 2");
        if (id == 0) return _p0;
        if (id == 1) return _p1;
        if (id == 2) return _p2;
        return Vector3();
    }

    Vector3 Triangle::normal() const {
        return Vector3::cross(_p2 - _p0, _p1 - _p0).normalized();
    }

    Vector3 Triangle::gravity() const {
        return (_p0 + _p1 + _p2) / 3.0;
    }

    bool Triangle::intersect(const Ray& ray, double* tHit) const {
        Vector3 qVec;
        double u, v;

        Vector3 e1 = _p1 - _p0;
        Vector3 e2 = _p2 - _p0;
        Vector3 pVec = Vector3::cross(ray.direction(), e2);
        double det = Vector3::dot(e1, pVec);

        if (det > EPS) {
            Vector3 tVec = ray.origin() - _p0;
            u = Vector3::dot(tVec, pVec);
            if (u < 0.0 || u > det) return false;

            qVec = Vector3::cross(tVec, e1);
            v = Vector3::dot(ray.direction(), qVec);
            if (v < 0.0 || u + v > det) return false;
        } else if (det < -EPS) {
            Vector3 tVec = ray.origin() - _p0;
            u = Vector3::dot(tVec, pVec);
            if (u > 0.0 || u < det) return false;

            qVec = Vector3::cross(tVec, e1);
            v = Vector3::dot(tVec, e1);
            if (v > 0.0 || v < det) return false;
        }

        *tHit = Vector3::dot(e2, qVec) / det;
        return true;
    }

}  // namespace spica
