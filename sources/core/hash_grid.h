#ifndef _SPICA_HASH_GRID_H_
#define _SPICA_HASH_GRID_H_

#include <vector>

#include "../core/point3d.h"
#include "../core/bound3d.h"

namespace spica {

template <class T>
class HashGrid {
public:
    HashGrid();
    virtual ~HashGrid();

    /**
     * Construct hash grid
     * @param[in] points: points stored to the grid
     * @param[in] imageW: image width used to determine the initial grid size (option)
     * @param[in] imageH: image height used to determine the initial grid size (option)
     */
    void construct(std::vector<T>& points, const int imageW = -1, const int imageH = -1);

    //! Initialize grid
    void init(const int hashSize, const double hashScale, const Bound3d& bbox);

    //! Set point data for the cells inside the specifed bounding box
    void add(const T& p, const Point& boxMin, const Point& boxMax);

    //! Clear grid data
    void clear();

    const std::vector<T>& operator[](const Point& v) const;

private:
    // Private methods
    unsigned int hash(const int ix, const int iy, const int iz) const;

    // Private fields
    int _hashSize;
    Bound3d _bbox;
    double _hashScale;
    std::vector<std::vector<T> > _data;

};  // class HashGrid

}  // namespace spica

#include "hash_grid_detail.h"

#endif  // _SPICA_HASH_GRID_H_

