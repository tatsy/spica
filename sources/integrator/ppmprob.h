#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHOTON_MAPPING_H_
#define _SPICA_PHOTON_MAPPING_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "photon_map.h"
#include "integrator.h"

namespace spica {

/** Progressive photon mapping: a probabilistic approach
 *  @ingroup renderer_module
 */
class SPICA_EXPORTS PPMProbIntegrator : public SamplerIntegrator {
public:
    /** Constructor. */
    PPMProbIntegrator(const std::shared_ptr<const Camera>& camera,
                        const std::shared_ptr<Sampler>& sampler,
                        double alpha = 0.8);
    /** Destructor. */
    ~PPMProbIntegrator();

    void initialize(const Scene& scene,
                    const RenderParams& params,
                    Sampler& sampler) override;

    void loopStarted(const Scene& scene,
                     const RenderParams& params,
                     Sampler& sampler) override;

    void loopFinished(const Scene& scene,
                      const RenderParams& parmas,
                      Sampler& sampler) override;

    Spectrum Li(const Scene& scene,
                const RenderParams& params,
                const Ray& ray,
                Sampler& sampler,
                MemoryArena& arena,
                int depth = 0) const override;

private:
    // Private fields
    PhotonMap photonmap_;
    double    globalRadius_;
    const double alpha_;
};

}  // namespace spica

#endif  // _SPICA_PHOTON_MAPPING_H_
