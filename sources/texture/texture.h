#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TEXTURE_H_
#define _SPICA_TEXTURE_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {

    /**
     * Texture mapping interface.
     */
    class SPICA_EXPORTS TextureMapping2D {
    public:
        virtual ~TextureMapping2D() {}
        virtual Point2D map(const SurfaceInteraction& intr, Vector2D* dstdx,
                            Vector2D* dstdy) const = 0;
    };

    class UVMapping2D : public TextureMapping2D {
    public:
        explicit UVMapping2D(double su = 1.0, double sv = 1.0,
                             double du = 0.0, double dv = 0.0);
        Point2D map(const SurfaceInteraction& intr, Vector2D* dstdx,
                    Vector2D* dstdy) const override;

    private:
        const double su_, sv_, du_, dv_;
    };

    /**
     * Texture interface.
     */
    template <class T>
    class Texture {
    public:
        virtual T evaluate(const SurfaceInteraction& intr) const = 0;
        virtual ~Texture() {}
    };

}  // namespace spica

#endif  // _SPICA_TEXTURE_H_
