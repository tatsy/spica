#define SPICA_TRIANGLE_EXPORT
#include "triangle.h"

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

}  // namespace spica
