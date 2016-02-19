/**
 *  @defgroup random_module Random
 *  @brief Random number generators.
 */
#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SAMPLER_H_
#define _SPICA_SAMPLER_H_

#include <memory>

#include "random_base.h"
#include "../core/common.h"
#include "../core/uncopyable.h"
#include "../core/point2d.h"

namespace spica {

    /** Random sampler class.
     *  @ingroup random_module
     */
    class SPICA_EXPORTS Sampler : public Uncopyable {
    public:
        virtual ~Sampler() {}

        virtual double  get1D() = 0;
        virtual Point2D get2D() {
            return Point2D{ get1D(), get1D() };   
        }

        virtual std::unique_ptr<Sampler> clone(unsigned int seed = 0) const = 0;
    };

}  // namespace spica

#endif  // _SPICA_SAMPLER_H_
