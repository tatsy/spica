#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BOX_H_
#define _SPICA_BOX_H_

#include "../core/common.h"
#include "filter.h"

namespace spica {

class SPICA_EXPORTS BoxFilter : public Filter {
public:
    BoxFilter();
    explicit BoxFilter(const Vector2d& radius);
    BoxFilter(const BoxFilter&) = default;
    ~BoxFilter();

    BoxFilter& operator=(const BoxFilter&) = default;

    double evaluate(const Point2d& p) const override;
};

}  // namespace spica

#endif  // _SPICA_BOX_H_
