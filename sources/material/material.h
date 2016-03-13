#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MATERIAL_H_
#define _SPICA_MATERIAL_H_

#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {

class SPICA_EXPORTS Material {
public:
    virtual ~Material() {}
    virtual void setScatterFuncs(SurfaceInteraction* intr, MemoryArena& arena) const = 0;
    // static void bump(SurfaceInteraction* intr, const std::shared_ptr<Texture<double> > &bumpmap);

};  // class Material

}  // namespace spica

#endif  // _SPICA_MATERIAL_H_
