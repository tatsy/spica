#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FILTER_H_
#define _SPICA_FILTER_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "../math/vector2d.h"

namespace spica {

class SPICA_EXPORTS Filter {
public:
    // Public methods
    Filter();
    explicit Filter(const Vector2D& radius);
    Filter(const Filter&) = default;
    virtual ~Filter();

    Filter& operator=(const Filter&) = default;

    virtual double evaluate(const Point2D& p) const = 0;

private:
    // Private fields
    Vector2D radius_;
    Vector2D invRad_;

};  // class Filter

}  // namespace spica

#endif  // _SPICA_FILTER_H_
