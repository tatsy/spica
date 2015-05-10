#define SPICA_BBOX_EXPORT
#include "bbox.h"

#include <algorithm>

#include "../utils/common.h"

namespace spica {

    BBox::BBox()
        : _posMin()
        , _posMax()
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

