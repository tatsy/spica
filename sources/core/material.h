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

class SPICA_EXPORTS Material : public CObject {
public:
    virtual ~Material() {}
    virtual void setScatterFuncs(SurfaceInteraction* intr, MemoryArena& arena) const = 0;
    // static void bump(SurfaceInteraction* intr, const std::shared_ptr<Texture<double> > &bumpmap);
    virtual bool isSubsurface() const { return false; }

};  // class Material

}  // namespace spica

#endif  // _SPICA_MATERIAL_H_
