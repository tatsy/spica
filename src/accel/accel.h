/**
 * @defgroup accel_module Accel
 * @brief The class module for intersecting accelerating data structures
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ACCEL_H_
#define _SPICA_ACCEL_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_ACCEL_EXPORT
        #define SPICA_ACCEL_DLL __declspec(dllexport)
    #else
        #define SPICA_ACCEL_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_ACCEL_DLL
#endif

#include <memory>

#include "../core/forward_decl.h"
#include "accel_interface.h"

namespace spica {

    class SPICA_ACCEL_DLL Accelerator {
    private:
        std::shared_ptr<IAccel> _ptr;
        AccelType _type;

    public:
        Accelerator();
        Accelerator(AccelType type);
        Accelerator(const Accelerator& accel);

        ~Accelerator();

        Accelerator& operator=(const Accelerator& accel);

        void setAccelType(AccelType type);

        void construct(const std::vector<Triangle>& triangles);
        int  intersect(const Ray& ray, Hitpoint* hitpoint) const;

        explicit operator bool() const;
    };

}  // namespace spica

#endif  // _SPICA_ACCEL_H_
