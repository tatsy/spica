#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_FILTER_H_
#define _SPICA_FILTER_H_

#include "core/core.hpp"
#include "core/common.h"
#include "core/cobject.h"
#include "core/vector2d.h"

namespace spica {

class SPICA_EXPORTS Filter : public CObject {
public:
    // Public methods
    Filter();
    explicit Filter(const Vector2d& radius);
    Filter(const Filter&) = default;
    virtual ~Filter();

    Filter& operator=(const Filter&) = default;

    virtual double evaluate(const Point2d& p) const = 0;

protected:
    // Protected fields
    Vector2d radius_;
    Vector2d invRad_;

};  // class Filter

}  // namespace spica

#endif  // _SPICA_FILTER_H_
