#ifndef _SPICA_HASH_GRID_DETAIL_H_
#define _SPICA_HASH_GRID_DETAIL_H_

#include <algorithm>

namespace spica {

    template <class Ty>
    HashGrid<Ty>::HashGrid()
        : _hashSize(-1)
        , _bbox()
        , _hashScale(0.0)
        , _data()
    {
    }

    template <class Ty>
    HashGrid<Ty>::~HashGrid()
    {
    }

    template <class Ty>
    void HashGrid<Ty>::construct(std::vector<Ty>& points, const int imageW, const int imageH) {
        
    }

    template <class Ty>
    void HashGrid<Ty>::init(const int hashSize, const double hashScale, const BBox& bbox) {
        this->_hashSize = hashSize;
        this->_hashScale = hashScale;
        this->_bbox = bbox;
        this->_data.resize(hashSize);
    }

    template <class Ty>
    void HashGrid<Ty>::add(const Ty& p, const Vector3D& boxMin, const Vector3D& boxMax) {
        const Vector3D bMin = (boxMin - _bbox.posMin()) * _hashScale;
        const Vector3D bMax = (boxMax - _bbox.posMin()) * _hashScale;

        const int minZ = std::abs(static_cast<int>(bMin.z()));
        const int maxZ = std::abs(static_cast<int>(bMax.z()));
        const int minY = std::abs(static_cast<int>(bMin.y()));
        const int maxY = std::abs(static_cast<int>(bMax.y()));
        const int minX = std::abs(static_cast<int>(bMin.x()));
        const int maxX = std::abs(static_cast<int>(bMax.x()));
        for (int iz = minZ; iz <= maxZ; iz++) {
            for (int iy = minY; iy <= maxY; iy++) {
                for (int ix = minX; ix <= maxX; ix++) {
                    unsigned int h = hash(ix, iy, iz);
                    _data[h].push_back(p);
                }
            }
        }
    }

    template <class Ty>
    void HashGrid<Ty>::clear() {
        _data.clear();
    }

    template <class Ty>
    unsigned int HashGrid<Ty>::hash(const int ix, const int iy, const int iz) const {
        Assertion(_hashSize > 0, "hash size is not initialized");
        return (unsigned int)((ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791)) % _hashSize;
    }

    template <class Ty>
    typename const std::vector<Ty>& HashGrid<Ty>::operator[](const Vector3D& v) const {
        Vector3D b = (v - _bbox.posMin()) * _hashScale;
        const int ix = std::abs(static_cast<int>(b.x()));
        const int iy = std::abs(static_cast<int>(b.y()));
        const int iz = std::abs(static_cast<int>(b.z()));
        unsigned int hashval = hash(ix, iy, iz);
        return _data[hash(ix, iy, iz)];
    }

}

#endif  // _SPICA_HASH_GRID_DETAIL_H_

