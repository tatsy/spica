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
    private:
        Vector3D _center = { 0.0, 0.0, 0.0 };
        double   _radius = 0.0;

    public:
        Sphere();
        Sphere(const Vector3D& center, double radius);
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

        inline Vector3D center() const { return _center; }
        inline double   radius() const { return _radius; }

    };  // class Sphere

}  // namespace spica

#endif  // SPICA_SPHERE_H_
