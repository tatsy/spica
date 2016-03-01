#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_DISK_H_
#define _SPICA_DISK_H_

#include <vector>

#include "shape.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"

namespace spica {

/** Disk class
 *  @ingroup shape_module
 */
class SPICA_EXPORTS Disk : public Shape {
public:
    Disk();
    Disk(const Point3d& center, const Normal3d& normal, double radius,
         const Transform& objectToWorld = Transform());
    Disk(const Disk& disk);
    ~Disk();

    Disk& operator=(const Disk& disk);

    bool intersect(const Ray& ray, double* tHit,
                   SurfaceInteraction* isect) const override;

    Bounds3d objectBound() const override;

    double area() const override;

    std::vector<Triangle> triangulate() const override;

    inline Point3d  center() const { return center_; }
    inline Normal3d normal() const { return normal_; }
    inline double   radius() const { return radius_; }
    
private:
    Point3d  center_ = { 0.0, 0.0, 0.0 };
    Normal3d normal_ = { 0.0, 0.0, 0.0 };
    double   radius_ = 0.0;

};  // class Disk

}  // namespace spica

#endif  // _SPICA_DISK_H_
