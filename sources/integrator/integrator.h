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
    Integrator(std::shared_ptr<Camera>& camera);
    virtual ~Integrator();
    virtual void render(const Scene& scene,
                        const RenderParameters& params) const = 0;

protected:
    std::shared_ptr<Camera>& camera_;
};

}  // namespace spica

#endif  // _SPICA_INTEGRATOR_H_
