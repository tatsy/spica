#define SPICA_BBOX_EXPORT
#include "bbox.h"

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

}  // namespace spica

