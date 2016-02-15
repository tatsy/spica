#define SPICA_API_EXPORT
#include "quad.h"

#include "../core/interaction.h"
#include "../renderer/ray.h"
#include "../math/vector3d.h"

#include "triangle.h"

namespace spica {
    
Quad::Quad()
    : Shape{ Transform(), ShapeType::Quad }
    , points_{} {
}

Quad::Quad(const Point& v0, const Point& v1,
            const Point& v2, const Point& v3,
            const Transform& objectToWorld)
    : Shape{ objectToWorld, ShapeType::Quad }
    , points_{ v0, v1, v2, v3 } {
}

Quad::Quad(const Quad& quad)
    : Quad{} {
    this->operator=(quad);
}

Quad::~Quad() {
}

Quad& Quad::operator=(const Quad& quad) {
    Shape::operator=(quad);
    this->points_ = quad.points_;
    return *this;
}

Point Quad::operator[](int id) const {
    Assertion(0 <= id && id <= 3,
                "Point ID must be in between 0 and 3 !!");
    return points_[id];
}


bool Quad::intersect(const Ray& ray, double* tHit, SurfaceInteraction* isect) const {
    if (tr(0, 1, 2).intersect(ray, tHit, isect)) {
        return true;
    }

    if (tr(0, 2, 3).intersect(ray, tHit, isect)) {
        return true;
    }

    return false;
}

Interaction Quad::sample(const Interaction& isect, const Point2D& rands) const {
    // TODO: Implement
    return {};
}

double Quad::area() const {
    return tr(0, 1, 2).area() + tr(0, 2, 3).area();
}

std::vector<Triangle> Quad::triangulate() const {
    std::vector<Triangle> tris(2);
    tris[0] = tr(0, 1, 2);
    tris[1] = tr(0, 2, 3);
    return std::move(tris);
}

Bound3d Quad::objectBound() const {
    Bound3d b;
    for (int i = 0; i < 4; i++) {
        b.merge(points_[i]);
    }
    return b;
}

Triangle Quad::tr(int i, int j, int k) const {
    return Triangle(points_[i], points_[j], points_[k]);
}

}  // namespace spica
