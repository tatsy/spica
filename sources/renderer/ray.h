#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_RAY_H_
#define SPICA_RAY_H_

#include "../core/common.h"
#include "../math/vector2d.h"
#include "../math/vector3d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../core/spectrum.h"

namespace spica {

/**
 * Ray class.
 */
class SPICA_EXPORTS Ray {
public:
    // Public methods
    Ray();
    Ray(const Point& origin, const Vector3D& direction, double maxDist = INFTY);
    Ray(const Ray& ray);
    virtual ~Ray();

    Ray& operator=(const Ray& ray);

    inline Point    org()     const { return org_; }
    inline Vector3D dir()     const { return dir_; }
    inline Vector3D invdir()  const { return invdir_; }
    inline double   maxDist() const { return maxDist_; }
    inline void     setMaxDist(double maxDist) { maxDist_ = maxDist; }

private:
    // Private methods
    void calcInvdir();

    // Private fields
    Point    org_     = { 0.0, 0.0, 0.0 };
    Vector3D dir_     = { 0.0, 0.0, 0.0 };
    Vector3D invdir_  = { INFTY, INFTY, INFTY };
    double   maxDist_ = INFTY;
};

}  // namespace spica

#endif  // SPICA_RAY_H_
