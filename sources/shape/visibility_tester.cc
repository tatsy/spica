#define SPICA_API_EXPORT
#include "visibility_tester.h"

namespace spica {

    VisibilityTester::VisibilityTester()
        : p1_{}
        , p2_{} {
    }

    VisibilityTester::VisibilityTester(const Interaction& p1,
                                       const Interaction& p2)
        : p1_{ p1 }
        , p2_{ p2 } {
    }

    bool VisibilityTester::unoccluded(const Scene& scene) const {
        // TODO: Implement!!    
        return true;
    }

}  // namespace spica

