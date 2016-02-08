#define SPICA_API_EXPORT
#include "quad.h"

#include "../math/vector3d.h"
#include "../renderer/ray.h"

#include "trimesh.h"

namespace spica {
    
    Quad::Quad()
        : IShape{ShapeType::Quad}
        , _points{} {
    }

    Quad::Quad(const Vector3D& v0, const Vector3D& v1,
               const Vector3D& v2, const Vector3D& v3)
        : IShape{ShapeType::Quad}
        , _points{v0, v1, v2, v3} {
    }

    Quad::Quad(const Quad& quad)
        : Quad{} {
        this->operator=(quad);
    }

    Quad::~Quad() {
    }

    Quad& Quad::operator=(const Quad& quad) {
        this->_points = quad._points;
        return *this;
    }

    bool Quad::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        if (tr(0, 1, 2).intersect(ray, hitpoint)) {
            return true;
        }

        if (tr(0, 2, 3).intersect(ray, hitpoint)) {
            return true;
        }

        return false;
    }

    double Quad::area() const {
        return tr(0, 1, 2).area() + tr(0, 2, 3).area();
    }

    Vector3D Quad::get(int id) const {
        Assertion(0 <= id && id <= 3,
                  "Point ID must be in between 0 and 3 !!");
        return _points[id];
    }

    Vector3D Quad::operator[](int id) const {
        Assertion(0 <= id && id <= 3,
                  "Point ID must be in between 0 and 3 !!");
        return _points[id];        
    }

    Trimesh Quad::triangulate() const {
        std::vector<Triangle> tris(2);
        tris[0] = tr(0, 1, 2);
        tris[1] = tr(0, 2, 3);
        return std::move(Trimesh{tris});
    }

    Vector3D Quad::normal() const {
        const Triangle t1 = tr(0, 1, 2);
        const Triangle t2 = tr(0, 2, 3);
        const double a1 = t1.area();
        const double a2 = t2.area();
        return (a1 / (a1 + a2)) * t1.normal() + (a2 / (a1 + a2)) * t2.normal();
    }

    Triangle Quad::tr(int i, int j, int k) const {
        return Triangle(_points[i], _points[j], _points[k]);
    }

}  // namespace spica
