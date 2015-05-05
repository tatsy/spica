#define SPICA_QUAD_EXPORT
#include "quad.h"

namespace spica {
    
    Quad::Quad()
        : _t0()
        , _t1()
    {
    }

    Quad::Quad(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3)
        : _t0(v0, v1, v2)
        , _t1(v2, v3, v0)
    {
    }

    Quad::Quad(const Quad& quad)
        : _t0()
        , _t1()
    {
        operator=(quad);
    }

    Quad::~Quad()
    {
    }

    Quad& Quad::operator=(const Quad& quad) {
        this->_t0 = quad._t0;
        this->_t1 = quad._t1;
        return *this;
    }

    bool Quad::intersect(const Ray& ray, double* tHit) const {
        if (_t0.intersect(ray, tHit)) {
            return true;
        }

        if (_t1.intersect(ray, tHit)) {
            return true;
        }

        return false;
    }

}  // namespace spica
