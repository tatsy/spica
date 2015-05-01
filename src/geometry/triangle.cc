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

    Vector3 Triangle::gravity() const {
        return (_p0 + _p1 + _p2) / 3.0;
    }

}  // namespace spica
