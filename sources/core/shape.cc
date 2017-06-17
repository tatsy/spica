#define SPICA_API_EXPORT
#include "shape.h"

#include "core/ray.h"
#include "core/interaction.h"

namespace spica {

Shape::Shape()
    : objectToWorld_{}
    , worldToObject_{}
    , type_{ ShapeType::None } {
}

Shape::Shape(const Transform& objectToWorld, ShapeType type)
    : objectToWorld_{ objectToWorld }
    , worldToObject_{ objectToWorld.inverted() }
    , type_{ type } {
}

Shape::Shape(const Shape& s)
    : Shape{ s.objectToWorld_, s.type_ } {
}

Shape::~Shape() {
}

Shape& Shape::operator=(const Shape& s) {
    this->objectToWorld_ = s.objectToWorld_;
    this->worldToObject_ = s.worldToObject_;
    this->type_          = s.type_;
    return *this;
}

double Shape::pdf(const Interaction& pObj) const {
    return 1.0 / area();
}

double Shape::pdf(const Interaction& pObj, const Vector3d& wi) const {
    Ray ray = pObj.spawnRay(wi);
    double tHit;
    SurfaceInteraction isect;
    if (!intersect(ray, &tHit, &isect)) return 0.0;

    double ret = (pObj.pos() - isect.pos()).squaredNorm() / (vect::absDot(isect.normal(), -wi) * area());
    if (std::isinf(ret)) ret = 0.0;
    return ret;
}

Interaction Shape::sample(const Interaction& ref, const Point2d& rands) const {
    return sample(rands);
}

Bounds3d Shape::worldBound() const {
    return objectToWorld_.apply(objectBound());
}

}  // namespace spica
