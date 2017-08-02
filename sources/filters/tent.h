#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TENT_H_
#define _SPICA_TENT_H_

#include "core/common.h"
#include "core/filter.h"
#include "core/renderparams.h"

namespace spica {

class SPICA_EXPORTS TentFilter : public Filter {
public:    
    TentFilter();
    explicit TentFilter(RenderParams &params);
    explicit TentFilter(const Vector2d& radius);
    TentFilter(const TentFilter&) = default;
    ~TentFilter();

    TentFilter& operator=(const TentFilter&) = default;

    double evaluate(const Point2d& p) const override;
};

SPICA_EXPORT_PLUGIN(TentFilter, "Tent reconstruction filter");

}  // namespace spica

#endif  // _SPICA_TENT_H_
