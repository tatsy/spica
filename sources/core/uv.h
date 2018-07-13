#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_UV_H_
#define _SPICA_UV_H_

#include <memory>

#include "../core/common.h"
#include "../core/spectrum.h"
#include "texture.h"

namespace spica {

    class SPICA_EXPORTS UVTexture : public Texture<Spectrum> {
    public:
        UVTexture(std::unique_ptr<TextureMapping2D> mapping);

        Spectrum evaluate(const SurfaceInteraction& intr) const override;

    private:

    };

}  // namespace spica

#endif  // _SPICA_UV_H_
