#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_SPHERE_H_
#define SPICA_SPHERE_H_

#include "shape_interface.h"
#include "../math/vector3d.h"
#include "../renderer/ray.h"

namespace spica {

    /** Sphere class
     *  @ingroup shape_module
     */
    class SPICA_EXPORTS Sphere : public IShape {
    public:
        Sphere();
        Sphere(const Point& center, double radius);
        Sphere(const Sphere& sphere);
        ~Sphere();

        Sphere& operator=(const Sphere& sphere);

        /** Compute intersection with ray
         *  @param[in] ray: a ray casted to the sphere
         *  @param[out] hitpoint: if ray hits to the sphere, intersecting point is inserted
         *  @return ray hits the sphere or not
         */
        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        Trimesh triangulate() const override;

        inline Point  center() const { return center_; }
        inline double radius() const { return radius_; }

    private:
        Point  center_ = { 0.0, 0.0, 0.0 };
        double radius_ = 0.0;

    };  // class Sphere

}  // namespace spica

#endif  // SPICA_SPHERE_H_
