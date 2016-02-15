/**
 *  @defgroup renderer_module Renderers
 *  @brief Renderer implementations
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _RENDERER_INTERFACE_H_
#define _RENDERER_INTERFACE_H_

#include <memory>

#include "../image/image.h"
#include "../core/uncopyable.h"
#include "../core/forward_decl.h"

namespace spica {

    /** Strictly-typed enum for renderer types.
     */
    enum class RendererType : int {
        None      = 0x00,  /**< None. */
        PathTrace,         /**< Unidirectional path tracing. */
        BDPT,              /**< Bidirectional path tracing. */
        Metropolis,        /**< Metropolis light transport. */
        PhotonMap,         /**< Probabilistic progressive photon mapping. */
        SPPM,              /**< Stochastic progressive photon mapping */
    };

    /** Interface for renderer implementations
     *  @ingroup renderer_module
     */
    class SPICA_EXPORTS IRenderer : private Uncopyable {
        
    protected:
        Image _result;
        // std::unique_ptr<SubsurfaceIntegrator> _integrator;
        RendererType _type;

    public:
        IRenderer(RendererType type);
        virtual ~IRenderer();

        virtual void render(const Scene& scene, const Camera& camera, 
                            const RenderParameters& params) = 0;

        const Image& result() const { return _result; }
        inline RendererType type() const { return _type; }
    };

}  // namespace spica

#endif  // _RENDERER_INTERFACE_H_
