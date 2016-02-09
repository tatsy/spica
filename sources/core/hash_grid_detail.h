#ifndef _SPICA_HASH_GRID_DETAIL_H_
#define _SPICA_HASH_GRID_DETAIL_H_

#include <algorithm>

namespace spica {

    template <class T>
    HashGrid<T>::HashGrid()
        : _hashSize(-1)
        , _bbox()
        , _hashScale(0.0)
        , _data()
    {
    }

    template <class T>
    HashGrid<T>::~HashGrid()
    {
    }

    template <class T>
    void HashGrid<T>::construct(std::vector<T>& points, const int imageW, const int imageH) {
        
    }

    template <class T>
    void HashGrid<T>::init(const int hashSize, const double hashScale, const BBox& bbox) {
        this->_hashSize = hashSize;
        this->_hashScale = hashScale;
        this->_bbox = bbox;
        this->_data.resize(hashSize);
    }

    template <class T>
    void HashGrid<T>::add(const T& p, const Point& boxMin, const Point& boxMax) {
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

    template <class T>
    void HashGrid<T>::clear() {
        _data.clear();
    }

    template <class T>
    unsigned int HashGrid<T>::hash(const int ix, const int iy, const int iz) const {
        Assertion(_hashSize > 0, "hash size is not initialized");
        return (unsigned int)((ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791)) % _hashSize;
    }

    template <class T>
    const typename std::vector<T>& HashGrid<T>::operator[](const Point& v) const {
        Vector3D b = (v - _bbox.posMin()) * _hashScale;
        const int ix = std::abs(static_cast<int>(b.x()));
        const int iy = std::abs(static_cast<int>(b.y()));
        const int iz = std::abs(static_cast<int>(b.z()));
        return _data[hash(ix, iy, iz)];
    }

}  // namespace spica

#endif  // _SPICA_HASH_GRID_DETAIL_H_

