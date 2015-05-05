#ifndef _SPICA_QUAD_H_
#define _SPICA_QUAD_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_QUAD_EXPORT
        #define SPICA_QUAD_DLL __declspec(dllexport)
    #else
        #define SPICA_QUAD_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_QUAD_DLL
#endif

#include "../utils/vector3.h"
#include "triangle.h"

namespace spica {

    class SPICA_QUAD_DLL Quad {
    private:
        Triangle _t0;
        Triangle _t1;

    public:
        Quad();
        Quad(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3);
        Quad(const Quad& quad);
        ~Quad();

        Quad& operator=(const Quad& quad);

        bool intersect(const Ray& ray, double* tHit) const;

        inline Vector3 p0() const { return _t0.p0(); }
        inline Vector3 p1() const { return _t0.p1(); }
        inline Vector3 p2() const { return _t0.p2(); }
        inline Vector3 p3() const { return _t1.p1(); }
    };

}  // namespace spica

#endif  // _SPICA_QUAD_H_
