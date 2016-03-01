#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VISIBILITY_TESTER_H_
#define _SPICA_VISIBILITY_TESTER_H_

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/interaction.h"

namespace spica {

    class SPICA_EXPORTS VisibilityTester {
    public:
        VisibilityTester();
        VisibilityTester(const Interaction& p1, const Interaction& p2);

        bool unoccluded(const Scene& scene) const;
        Spectrum transmittance(const Scene& scene, Sampler& smapler) const;

        inline const Interaction& p1() const { return p1_; }
        inline const Interaction& p2() const { return p2_; }

    private:
        Interaction p1_, p2_;
    };

}  // namespace spica

#endif  // _SPICA_VISIBILITY_TESTER_H_
