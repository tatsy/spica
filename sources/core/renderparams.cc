#define SPICA_API_EXPORT
#include "renderparams.h"

namespace spica {

// -----------------------------------------------------------------------------
// Other method definitions
// -----------------------------------------------------------------------------

RenderParams::RenderParams()
    : table_{} {
    // Set default parameters
    set("MAX_BOUNCES", 32);
    set("NUM_SAMPLES", 32);
    set("CAST_PHOTONS", 500000);
}

void RenderParams::clear() {
    table_.clear();
}

}  // namespace spica
