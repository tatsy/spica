#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_DIFFUSE_H_
#define _SPICA_DIFFUSE_H_

#include "core/core.hpp"
#include "core/common.h"
#include "core/spectrum.h"
#include "core/render.hpp"
#include "core/material.h"

namespace spica {

class SPICA_EXPORTS Diffuse : public SurfaceMaterial {
public:
    Diffuse(const std::shared_ptr<Texture<Spectrum>>& Kd,
            const std::shared_ptr<Texture<Spectrum>>& bumpMap = nullptr);
    Diffuse(RenderParams &params);

    void setScatterFuncs(SurfaceInteraction* intr,
                         MemoryArena& arena) const override;

private:
    std::shared_ptr<Texture<Spectrum>> Kd_;
    std::shared_ptr<Texture<Spectrum>> bumpMap_;
};

SPICA_EXPORT_PLUGIN(Diffuse, "Lambertian diffuse surface");

}  // namespace spica

#endif  // _SPICA_DIFFUSE_H_
