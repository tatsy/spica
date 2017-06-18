#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_RAY_H_
#define SPICA_RAY_H_

#include <memory>

#include "core/common.h"
#include "core/vector2d.h"
#include "core/vector3d.h"
#include "core/point3d.h"
#include "core/normal3d.h"
#include "core/spectrum.h"

namespace spica {

/**
 * Ray class.
 */
class SPICA_EXPORTS Ray {
public:
    // Public methods
    Ray();
    Ray(const Point3d& origin, const Vector3d& direction, double maxDist = INFTY);
    Ray(const Ray& ray);
    virtual ~Ray();

    Ray& operator=(const Ray& ray);

    //! Return the proceeded position of origin with distance "t".
    Point3d proceeded(double t) const;

    inline Point3d  org()     const { return org_; }
    inline Vector3d dir()     const { return dir_; }
    inline Vector3d invdir()  const { return invdir_; }
    inline double   maxDist() const { return maxDist_; }
    inline void     setMaxDist(double maxDist) { maxDist_ = maxDist; }

private:
    // Private methods
    void calcInvdir();

    // Private fields
    Point3d  org_     = { 0.0, 0.0, 0.0 };
    Vector3d dir_     = { 0.0, 0.0, 0.0 };
    Vector3d invdir_  = { INFTY, INFTY, INFTY };
    double   maxDist_ = INFTY;
};

}  // namespace spica

#endif  // SPICA_RAY_H_
