#ifndef SPICA_PLANE_H_
#define SPICA_PLANE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PLANE_EXPORT
        #define SPICA_PLANE_DLL __declspec(dllexport)
    #else
        #define SPICA_PLANE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_PLANE_DLL
#endif

#include "shape_interface.h"
#include "../math/vector3d.h"

namespace spica {

    /** Plane class
     *  @ingroup shape_class
     */
    class SPICA_PLANE_DLL Plane : public IShape {
    protected:
        double   _distance = 0.0;
        Vector3D _normal   = { 0.0, 0.0, 0.0 };

    public:
        Plane();
        Plane(double distance, const Vector3D& normal);
        Plane(const Plane& plane);
        virtual ~Plane();

        Plane& operator=(const Plane& plane);

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        std::vector<Triangle> triangulate() const override;

        inline double distance() const { return _distance; }
        inline Vector3D normal() const { return _normal; }
    };
}

#endif  // SPICA_PLANE_H_
