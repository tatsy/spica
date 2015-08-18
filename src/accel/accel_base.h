#ifndef _SPICA_ACCEL_BASE_H_
#define _SPICA_ACCEL_BASE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_ACCEL_BASE_EXPORT
        #define SPICA_ACCEL_BASE_DLL __declspec(dllexport)
    #else
        #define SPICA_ACCEL_BASE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_ACCEL_BASE_DLL
#endif

#include <vector>
#include <map>

#include "../utils/common.h"
#include "../renderer/ray.h"
#include "../geometry/triangle.h"
#include "../geometry/bbox.h"

namespace spica {

    class AccelBase {
    protected:
        typedef std::pair<Triangle, int> TriangleWithID;

        struct AxisComparator {
            int dim;
            explicit AxisComparator(int dim_ = 0)
                : dim(dim_)
            {
                Assertion(0 <= dim_ && dim_ <= 2, "Dimension must be between 0 and 2");
            }

            bool operator()(const TriangleWithID& t1, const TriangleWithID& t2) const {
                return t1.first.gravity().get(dim) < t2.first.gravity().get(dim);
            }
        };


    public:
        virtual ~AccelBase() {}
        virtual void construct(const std::vector<Triangle>& triangles) = 0;
        virtual int  intersect(const Ray& ray, Hitpoint* hitpoint) const = 0;

    protected:
        static BBox enclosingBox(const std::vector<TriangleWithID>& triangles);
    };

}

#endif  // _SPICA_ACCEL_BASE_H_
