#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MIS_H_
#define _SPICA_MIS_H_

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"

namespace spica {

namespace mis {

SPICA_EXPORTS Spectrum uniformSampleOneLight(const Interaction& intr,
                                             const Scene& scene,
                                             MemoryArena& arena,
                                             Sampler& sampler);

SPICA_EXPORTS Spectrum estimateDirectLight(const Interaction& intr,
                                           const Point2D& randShade,
                                           const Light& light,
                                           const Point2D& randLight,
                                           const Scene& scene, Sampler& sampler,
                                           MemoryArena& arena,
                                           bool specular = false);

SPICA_EXPORTS double powerHeuristic(int nf, double fPdf, int ng, double gPdf);

SPICA_EXPORTS Distribution1D calcLightPowerDistrib(const Scene& scene);

}  // namespace mis

}  // namespace spica

#endif  // _SPICA_MIS_H_
