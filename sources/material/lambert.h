#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LAMBERT_H_
#define _SPICA_LAMBERT_H_

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"

#include "material.h"

#include "../texture/texture.h"

namespace spica {

class SPICA_EXPORTS LambertianMaterial : public Material {
public:
    LambertianMaterial(const std::shared_ptr<Texture<Spectrum> >& Kd,
                       const std::shared_ptr<Texture<double> >& bundMap = nullptr);

    void setScatterFuncs(SurfaceInteraction* intr,
                         MemoryArena& arena) const override;

private:
    std::shared_ptr<Texture<Spectrum> > Kd_;
    std::shared_ptr<Texture<double> > bumpMap_;
};

}  // namespace spica

#endif  // _SPICA_LAMBERT_H_
