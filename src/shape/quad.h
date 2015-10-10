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

#include <array>

#include "shape_interface.h"
#include "../math/vector3d.h"

namespace spica {

    /** Quadrangle class
     *  @ingroup shape_module
     */
    class SPICA_QUAD_DLL Quad : public IShape {
    private:
        std::array<Vector3D, 4> _points;

    public:
        Quad();
        Quad(const Vector3D& v0, const Vector3D& v1, const Vector3D& v2, const Vector3D& v3);
        Quad(const Quad& quad);
        ~Quad();

        Quad& operator=(const Quad& quad);

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        double area() const override;

        std::vector<Triangle> triangulate() const override;

        Vector3D operator[](int id) const;
        Vector3D get(int id) const;

        Vector3D normal() const;

    private:
        Triangle tr(int i, int j, int k) const;
    };

}  // namespace spica

#endif  // _SPICA_QUAD_H_
