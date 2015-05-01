#ifndef _SPICA_TRIANGLE_H_
#define _SPICA_TRIANGLE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_TRIANGLE_EXPORT
        #define SPICA_TRIANGLE_DLL __declspec(dllexport)
    #else
        #define SPICA_TRIANGLE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_TRIANGLE_DLL
#endif

#include "../renderer/ray.h"
#include "../utils/vector3.h"

namespace spica {

    class SPICA_TRIANGLE_DLL Triangle {
    private:
        Vector3 _p0;
        Vector3 _p1;
        Vector3 _p2;

    public:
        Triangle();
        Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2);
        Triangle(const Triangle& tri);
        ~Triangle();

        Triangle& operator=(const Triangle& tri);

        Vector3 gravity() const;
        Vector3 p(int id) const;

        Vector3 normal() const;

        // Compute ray-triangle intersection with Tomas Moller's algorithm
        bool intersect(const Ray& ray, double* tHit) const;

        inline Vector3 p0() const { return _p0; }
        inline Vector3 p1() const { return _p1; }
        inline Vector3 p2() const { return _p2; }
    };

}

#endif  // _SPICA_TRIANGLE_H_
