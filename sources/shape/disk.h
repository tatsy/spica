#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_DISK_H_
#define _SPICA_DISK_H_

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
        Disk(const Point& center, const Normal& normal, double radius);
        Disk(const Disk& disk);
        ~Disk();

        Disk& operator=(const Disk& disk);

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        Trimesh triangulate() const override;

        inline Point  center() const { return center_; }
        inline Normal normal() const { return normal_; }
        inline double radius() const { return radius_; }
    
    private:
        Point  center_ = { 0.0, 0.0, 0.0 };
        Normal normal_ = { 0.0, 0.0, 0.0 };
        double radius_ = 0.0;
    };

}  // namespace spica

#endif  // _SPICA_DISK_H_
