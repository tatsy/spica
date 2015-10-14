#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIANGLE_DATA_H_
#define _SPICA_TRIANGLE_DATA_H_

#include "../core/triplet.h"

namespace spica {

    class TriangleData {
    private:
        Triplet _trip;
        bool _isTextured;

    public:
        TriangleData()
            : _trip()
            , _isTextured(false) {
        }

        TriangleData(int i, int j, int k, bool isTextured = false)
            : _trip(i, j, k)
            , _isTextured(isTextured) {
        }

        TriangleData(const TriangleData& tri)
            : _trip(tri._trip)
            , _isTextured(tri._isTextured) {
        }

        ~TriangleData() {
        }

        TriangleData& operator=(const TriangleData& tri) {
            this->_trip = tri._trip;
            this->_isTextured = tri._isTextured;
            return *this;
        }

        int operator[](int d) const {
            Assertion(0 <= d && d <= 2, "Dimension must be between 0 and 2.");
            return _trip[d];
        }

        inline bool isTextured() const { return _isTextured; }
    };

}  // namespace spica

#endif  // _SPICA_TRIANGLE_DATA_H_
