#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TENT_H_
#define _SPICA_TENT_H_

#include "../core/common.h"
#include "filter.h"

namespace spica {

class SPICA_EXPORTS TentFilter : public Filter {
public:    
    TentFilter();
    TentFilter(const Vector2D& radius);
    TentFilter(const TentFilter&) = default;
    ~TentFilter();

    TentFilter& operator=(const TentFilter&) = default;

    double evaluate(const Point2D& p) const override;
};

}  // namespace spica

#endif  // _SPICA_TENT_H_
