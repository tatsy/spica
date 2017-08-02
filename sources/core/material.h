#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MATERIAL_H_
#define _SPICA_MATERIAL_H_

#include <memory>

#include "core/common.h"
#include "core/core.hpp"
#include "core/cobject.h"

namespace spica {

class SPICA_EXPORTS SurfaceMaterial : public CObject {
public:
    virtual ~SurfaceMaterial();
    virtual void setScatterFuncs(SurfaceInteraction* intr, MemoryArena& arena) const = 0;
    static void bump(SurfaceInteraction* intr, const std::shared_ptr<Texture<double> > &bumpmap);
};

class SPICA_EXPORTS SubsurfaceMaterial : public CObject {
public:
    virtual ~SubsurfaceMaterial();
    virtual void setScatterFuncs(const SurfaceInteraction *intr, MemoryArena &arena) const = 0;
};

class SPICA_EXPORTS Material {
public:
    Material(const std::shared_ptr<SurfaceMaterial> &bsdf,
             const std::shared_ptr<SubsurfaceMaterial> &subsurface = nullptr);
    void setScatterFuncs(SurfaceInteraction *intr, MemoryArena &arena) const;
    bool isSubsurface() const { return static_cast<bool>(subsurface_); }

private:
    std::shared_ptr<SurfaceMaterial> bsdf_ = nullptr;
    std::shared_ptr<SubsurfaceMaterial> subsurface_ = nullptr;
};

}  // namespace spica

#endif  // _SPICA_MATERIAL_H_
