#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ACCELERATOR_H_
#define _SPICA_ACCELERATOR_H_

#include <vector>
#include <map>

#include "core/core.hpp"
#include "core/common.h"
#include "core/uncopyable.h"
#include "core/triangle.h"
#include "core/bounds3d.h"

#include "render/primitive.h"

namespace spica {

/** Interface for intersection test accelerators.
 *  @ingroup accel_module
 */
class SPICA_EXPORTS Accelerator : public Aggregate, Uncopyable {
public:
    // Public methods
    explicit Accelerator(const std::vector<std::shared_ptr<Primitive>>& primitives)
        : primitives_{ primitives } {}
    virtual ~Accelerator() {}
    virtual void construct() = 0;
    virtual Bounds3d worldBound() const override {
        return worldBound_;
    }

    inline const std::vector<std::shared_ptr<Primitive>>& primitives() const {
        return primitives_;
    }

protected:
    // Protected internal classes
    struct IndexedTriangle;
    struct AxisComparator;

    // Protected fields
    std::vector<std::shared_ptr<Primitive>> primitives_;
    Bounds3d   worldBound_;
};

}  // namespace spica

#endif  // _SPICA_ACCELERATOR_H_
