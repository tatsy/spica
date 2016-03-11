#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MIPMAP_H_
#define _SPICA_MIPMAP_H_

#include <vector>

#include "../core/common.h"
#include "../core/forward_decl.h"

#include "image.h"

namespace spica {

    enum class ImageWrap : int {
        Black,
        Clamp,
        Repeat,
    };

    class SPICA_EXPORTS MipMap {
    public:
        MipMap(const Image& image, ImageWrap imageWrap = ImageWrap::Repeat);

        Spectrum lookup(const Point2d& st, double width = 0.0) const;
        Spectrum lookup(const Point2d& st, const Vector2d& dstdx, const Vector2d& dstdy) const;

        inline int levels() const { return static_cast<int>(pyramid_.size()); }

    private:
        Spectrum bilinear(int level, const Point2d& st) const;
        Spectrum texel(int level, int s, int t) const;

        ImageWrap imageWrap_;
        std::vector<Image> pyramid_;
    };

}  // namespace spica

#endif  // _SPICA_MIPMAP_H_
