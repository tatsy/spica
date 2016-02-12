/**
 * @defgroup accel_module Accel
 * @brief The class module for intersecting accelerating data structures
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ACCEL_H_
#define _SPICA_ACCEL_H_

#include <memory>

#include "../core/forward_decl.h"
#include "accel_interface.h"

namespace spica {

    class SPICA_EXPORTS Accelerator {
    private:
        std::shared_ptr<AccelInterface> _ptr;
        AccelType _type;

    public:
        Accelerator();
        Accelerator(AccelType type);
        Accelerator(const Accelerator& accel);

        ~Accelerator();

        Accelerator& operator=(const Accelerator& accel);

        void setAccelType(AccelType type);

        void construct(const std::vector<Triangle>& triangles);
        bool intersect(const Ray& ray, SurfaceInteraction* isect) const;

        explicit operator bool() const;
    };

}  // namespace spica

#endif  // _SPICA_ACCEL_H_
