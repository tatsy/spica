#define SPICA_BBOX_EXPORT
#include "bbox.h"

#include <algorithm>

#include "../utils/common.h"

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

    BBox::BBox(const Vector3& posMin, const Vector3& posMax)
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

    void BBox::merge(const Vector3& v) {
        _posMin = Vector3::minimum(_posMin, v);
        _posMax = Vector3::maximum(_posMax, v);
    }

    void BBox::merge(const BBox& box) {
        _posMin = Vector3::minimum(_posMin, box._posMin);
        _posMax = Vector3::maximum(_posMax, box._posMax);
    }

    void BBox::merge(const Triangle& t) {
        for (int k = 0; k < 3; k++) {
            this->merge(t.p(k));
        }
    }

    bool BBox::inside(const Vector3& v) const {
        return (_posMin.x() <= v.x() && v.x() <= _posMax.x()) &&
               (_posMin.y() <= v.y() && v.y() <= _posMax.y()) &&
               (_posMin.z() <= v.z() && v.z() <= _posMax.z());
    }

    bool BBox::intersect(const Ray& ray, double* tMin, double* tMax) const {
        double xMin = (_posMin.x() - ray.origin().x()) * ray.invdir().x();
        double xMax = (_posMax.x() - ray.origin().x()) * ray.invdir().x();
        double yMin = (_posMin.y() - ray.origin().y()) * ray.invdir().y();
        double yMax = (_posMax.y() - ray.origin().y()) * ray.invdir().y();
        double zMin = (_posMin.z() - ray.origin().z()) * ray.invdir().z();
        double zMax = (_posMax.z() - ray.origin().z()) * ray.invdir().z();
        
        if (xMin > xMax) std::swap(xMin, xMax);
        if (yMin > yMax) std::swap(yMin, yMax);
        if (zMin > zMax) std::swap(zMin, zMax);

        *tMin = std::max(xMin, std::max(yMin, zMin));
        *tMax = std::min(xMax, std::min(yMax, zMax));

        if (*tMin > *tMax || (*tMin < 0.0 && *tMax < 0.0)) {
            return false;
        }
         
        if (*tMin < 0.0 && *tMax >= 0.0) {
            *tMin = *tMax;
            *tMax = INFTY;
        }
        return true;
    }

}  // namespace spica

