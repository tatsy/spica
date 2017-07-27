#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_INDEPENDENT_H_
#define _SPICA_INDEPENDENT_H_

#include "core/random.h"

namespace spica {

class SPICA_EXPORTS Independent : public Sampler {
public:
    explicit Independent(uint32_t seed);
    explicit Independent(const RenderParams &params);

    ~Independent();

    double get1D() override;

    std::unique_ptr<Sampler> clone(uint32_t seed = 0) const override;

private:
    Random random_;
};

SPICA_EXPORT_PLUGIN(Independent, "Random number generator with Mersenne twister.");

}  // namespace spica

#endif  // _SPICA_INDEPENDENT_H_
