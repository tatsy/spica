#define SPICA_API_EXPORT
#include "bbox.h"

#include <algorithm>

#include "../core/common.h"

#include "trimesh.h"

namespace spica {

    BBox::BBox()
        : IShape{ShapeType::BBox}
        , posMin_{  INFTY,  INFTY,  INFTY }
        , posMax_{ -INFTY, -INFTY, -INFTY } {
    }

    BBox::BBox(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
        : IShape{ShapeType::BBox}
        , posMin_{ minX, minY, minZ }
        , posMax_{ maxX, maxY, maxZ } {
    }

    BBox::BBox(const Point& posMin, const Point& posMax)
        : IShape{ShapeType::BBox}
        , posMin_{ posMin }
        , posMax_{ posMax } {
    }

    BBox::BBox(const BBox& box) 
        : BBox{} {
        this->operator=(box);
    }

    BBox::~BBox() {
    }

    BBox& BBox::operator=(const BBox& box) {
        IShape::operator=(box);
        this->posMin_ = box.posMin_;
        this->posMax_ = box.posMax_;
        return *this;
    }

    BBox BBox::fromTriangle(const Triangle& t) {
        BBox retval;
        retval.merge(t[0]);
        retval.merge(t[1]);
        retval.merge(t[2]);
        return retval;
    }

    void BBox::merge(const Point& v) {
        posMin_ = std::max(posMin_, v); //Vector3D::minimum(posMin_, v);
        posMax_ = Point::maximum(posMax_, v);
    }

    void BBox::merge(const BBox& box) {
        posMin_ = Point::minimum(posMin_, box.posMin_);
        posMax_ = Point::maximum(posMax_, box.posMax_);
    }

    BBox BBox::merge(const BBox& b1, const BBox& b2) {
        BBox retval = b1;
        retval.merge(b2);
        return retval;
    }

    void BBox::merge(const Triangle& t) {
        for (int k = 0; k < 3; k++) {
            this->merge(t[k]);
        }
    }

    bool BBox::inside(const Point& v) const {
        return (posMin_.x() <= v.x() && v.x() <= posMax_.x()) &&
               (posMin_.y() <= v.y() && v.y() <= posMax_.y()) &&
               (posMin_.z() <= v.z() && v.z() <= posMax_.z());
    }

    int BBox::maximumExtent() const {
        const Vector3D g = posMax_ - posMin_;
        const double gx = std::abs(g.x());
        const double gy = std::abs(g.y());
        const double gz = std::abs(g.z());
        if (gx >= gy && gx >= gz) return 0;
        if (gy >= gx && gy >= gz) return 1;
        return 2;
    }

    double BBox::area() const {
        const Vector3D g = posMin_ - posMax_;
        const double gx = std::abs(g.x());
        const double gy = std::abs(g.y());
        const double gz = std::abs(g.z());
        return 2.0 * (gx * gy + gy * gz + gz * gx);
    }

    bool BBox::intersect(const Ray& ray, double* tMin, double* tMax) const {
        double xMin = (posMin_.x() - ray.origin().x()) * ray.invdir().x();
        double yMin = (posMin_.y() - ray.origin().y()) * ray.invdir().y();
        double zMin = (posMin_.z() - ray.origin().z()) * ray.invdir().z();
        double xMax = (posMax_.x() - ray.origin().x()) * ray.invdir().x();
        double yMax = (posMax_.y() - ray.origin().y()) * ray.invdir().y();
        double zMax = (posMax_.z() - ray.origin().z()) * ray.invdir().z();
        
        if (xMin > xMax) std::swap(xMin, xMax);
        if (yMin > yMax) std::swap(yMin, yMax);
        if (zMin > zMax) std::swap(zMin, zMax);

        *tMin = std::max(xMin, std::max(yMin, zMin));
        *tMax = std::min(xMax, std::min(yMax, zMax));

        if (*tMin > *tMax || (*tMin < 0.0 && *tMax < 0.0)) {
            (*tMin) = INFTY;
            (*tMax) = INFTY;
            return false;
        }         
        return true;
    }

    bool BBox::intersect(const Ray& ray, Hitpoint* hitpoint) const {
        // TODO: this is STAB
        return true;
    }

    Trimesh BBox::triangulate() const {
        // TODO: this is STAB
        return Trimesh{};
    }

}  // namespace spica

