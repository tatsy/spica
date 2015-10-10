#define SPICA_BBOX_EXPORT
#include "bbox.h"

#include <algorithm>

#include "../core/common.h"

namespace spica {

    BBox::BBox()
        : _posMin(INFTY, INFTY, INFTY)
        , _posMax(-INFTY, -INFTY, -INFTY)
    {
    }

    BBox::BBox(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
        : _posMin(minX, minY, minZ)
        , _posMax(maxX, maxY, maxZ)
    {
    }

    BBox::BBox(const Vector3D& posMin, const Vector3D& posMax)
        : _posMin(posMin)
        , _posMax(posMax)
    {
    }

    BBox::BBox(const BBox& box) 
        : _posMin(box._posMin)
        , _posMax(box._posMax)
    {
    }

    BBox::~BBox()
    {
    }

    BBox& BBox::operator=(const BBox& box) {
        this->_posMin = box._posMin;
        this->_posMax = box._posMax;
        return *this;
    }

    BBox BBox::fromTriangle(const Triangle& t) {
        BBox retval;
        retval.merge(t[0]);
        retval.merge(t[1]);
        retval.merge(t[2]);
        return retval;
    }

    void BBox::merge(const Vector3D& v) {
        _posMin = Vector3D::minimum(_posMin, v);
        _posMax = Vector3D::maximum(_posMax, v);
    }

    void BBox::merge(const BBox& box) {
        _posMin = Vector3D::minimum(_posMin, box._posMin);
        _posMax = Vector3D::maximum(_posMax, box._posMax);
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

    bool BBox::inside(const Vector3D& v) const {
        return (_posMin.x() <= v.x() && v.x() <= _posMax.x()) &&
               (_posMin.y() <= v.y() && v.y() <= _posMax.y()) &&
               (_posMin.z() <= v.z() && v.z() <= _posMax.z());
    }

    int BBox::maximumExtent() const {
        const Vector3D g = _posMax - _posMin;
        const double gx = std::abs(g.x());
        const double gy = std::abs(g.y());
        const double gz = std::abs(g.z());
        if (gx >= gy && gx >= gz) return 0;
        if (gy >= gx && gy >= gz) return 1;
        return 2;
    }

    double BBox::area() const {
        const Vector3D g = _posMin - _posMax;
        const double gx = std::abs(g.x());
        const double gy = std::abs(g.y());
        const double gz = std::abs(g.z());
        return 2.0 * (gx * gy + gy * gz + gz * gx);
    }

    bool BBox::intersect(const Ray& ray, double* tMin, double* tMax) const {
        double xMin = (_posMin.x() - ray.origin().x()) * ray.invdir().x();
        double yMin = (_posMin.y() - ray.origin().y()) * ray.invdir().y();
        double zMin = (_posMin.z() - ray.origin().z()) * ray.invdir().z();
        double xMax = (_posMax.x() - ray.origin().x()) * ray.invdir().x();
        double yMax = (_posMax.y() - ray.origin().y()) * ray.invdir().y();
        double zMax = (_posMax.z() - ray.origin().z()) * ray.invdir().z();
        
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

}  // namespace spica

