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

        Sampler(Sampler&& sampler)
            : rng_{std::move(sampler.rng_)} {
        }

        Sampler& operator=(Sampler&& sampler) {
            this->rng_ = std::move(sampler.rng_);
            return *this;
        }

        double get1D() {
            return rng_->get1D();
        }

        Point2D get2D() {
            return { get1D(), get1D() };
        }

    private:
        // Private methods
        Sampler(RandomInterface* rng)
            : rng_{ rng } {
        }

        // Private fields
        std::unique_ptr<RandomInterface> rng_;

        // Friend classes.
        friend class Random;
        friend class Halton;
    };

}  // namespace spica

#endif  // _SPICA_SAMPLER_H_
