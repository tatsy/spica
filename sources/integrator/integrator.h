/**
 * @defgroup renderer_module Renderers
 * @brief Renderer implementations
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_INTEGRATOR_H_
#define _SPICA_INTEGRATOR_H_

#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"

#include "../core/uncopyable.h"

namespace spica {

/**
 * The integrator interface.
 */
class SPICA_EXPORTS Integrator : public Uncopyable {
public:
    // Public methods
    explicit Integrator(const std::shared_ptr<const Camera>& camera);
    virtual ~Integrator();
    virtual void render(const Scene& scene,
                        const RenderParameters& params) const = 0;

protected:
    std::shared_ptr<const Camera> camera_;

};  // class Integrator

/**
 * The sampler integrator interface.
 * @details
 * This interface class is for integrating with sampling.
 */
class SPICA_EXPORTS SamplerIntegrator : public Integrator {
public:
    // Public methods
    SamplerIntegrator(const std::shared_ptr<const Camera>& camera,
                      const std::shared_ptr<Sampler>& sampler);
    virtual ~SamplerIntegrator();
    virtual void render(const Scene& scene,
                        const RenderParameters& params) const;

    virtual Spectrum Li(const Scene& scene,
                        const RenderParameters& params,
                        const Ray& ray,
                        Sampler& sampler,
                        MemoryArena& arena,
                        int depth = 0) const = 0;

    Spectrum specularReflect(const Scene& scene,
                             const RenderParameters& params,
                             const Ray& ray,
                             const SurfaceInteraction& isect,
                             Sampler& sampler,
                             MemoryArena& arena,
                             int depth = 0) const;

    Spectrum specularTransmit(const Scene& scene,
                              const RenderParameters& params,
                              const Ray& ray,
                              const SurfaceInteraction& isect,
                              Sampler& sampler,
                              MemoryArena& arena,
                              int depth = 0) const;

private:
    // Private fields
    std::shared_ptr<Sampler> sampler_;

};  // class SamplerIntegrator

}  // namespace spica

#endif  // _SPICA_INTEGRATOR_H_
