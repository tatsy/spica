#define SPICA_API_EXPORT
#include "mipmap.h"

#include <algorithm>

#include "../core/point2d.h"

namespace spica {

    namespace {
    
        int roundUpPow2(int v) {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            return v + 1;
        }

        bool isPowerOf2(int v) {
            return (v && !(v & (v - 1)));
        }

        int log2Int(int v) {
            int ret = 0;
            while (v != 0) {
                ret++;
                v >>= 1;
            }
            return ret;
        }

    }  // anonymous namespace

    MipMap::MipMap(const Image& image, ImageWrap imageWrap)
        : imageWrap_{ imageWrap }
        , pyramid_{} {
        Point2i resolution(image.width(), image.height());
        Point2i resPow2(roundUpPow2(resolution[0]), roundUpPow2(resolution[1]));

        if (!isPowerOf2(resolution[0]) || !isPowerOf2(resolution[1])) {
            resolution = resPow2;            
        }

        int nLevels = 1 + log2Int(std::max(resolution[0], resolution[1]));
        pyramid_.resize(nLevels);

        pyramid_[0] = image;
        for (int i = 1; i < nLevels; i++) {
            int sRes = std::max(1, pyramid_[i - 1].width() / 2);
            int tRes = std::max(1, pyramid_[i - 1].height() / 2);
            pyramid_[i].resize(sRes, tRes);
            for (int y = 0; y < tRes; y++) {
                for (int x = 0; x < sRes; x++) {
                    pyramid_[i].pixel(x, y) = 0.25 * (
                            texel(i - 1, x * 2, y * 2) + 
                            texel(i - 1, x * 2 + 1, y * 2) + 
                            texel(i - 1, x * 2, y * 2 + 1) + 
                            texel(i - 1, x * 2 + 1, y * 2 + 1));
                }
            }
        }
    }

    Spectrum MipMap::lookup(const Point2d& st, double width) const {
        const double level = levels() - 1 + log2(std::max(width, 1.0e-8));

        if (level < 0) {
            return bilinear(0, st);
        } else if (level >= levels() - 1) {
            return texel(levels() - 1, 0, 0);
        } else {
            int l = static_cast<int>(level);
            double delta = level - l;
            return (1.0 - delta) * bilinear(level, st) + delta * bilinear(level + 1, st);
        }
    }

    Spectrum MipMap::bilinear(int level, const Point2d& st) const {
        level = clamp(level, 0, levels());
        const double s = st[0] * pyramid_[level].width()  - 0.5;
        const double t = st[1] * pyramid_[level].height() - 0.5;
        const int si = static_cast<int>(s);
        const int ti = static_cast<int>(t);
        const double ds = s - si;
        const double dt = t - ti;
        return (1.0 - ds) * (1.0 - dt) * texel(level, si, ti) +
               ds * (1.0 - dt) * texel(level, si + 1, ti) +
               (1.0 - ds) * dt * texel(level, si, ti + 1) +
                ds * dt * texel(level, si + 1, ti + 1);
    }

    Spectrum MipMap::texel(int level, int s, int t) const {
        Assertion(level < pyramid_.size(), "Level is too high!");
        const int w = pyramid_[level].width();
        const int h = pyramid_[level].height();
        switch (imageWrap_) {
        case ImageWrap::Repeat:
            s = (s % w + w) % w;
            t = (t % h + h) % h;
            break;

        case ImageWrap::Clamp:
            s = clamp(s, 0, w - 1);
            t = clamp(t, 0, h - 1);
            break;

        case ImageWrap::Black:
            if (s < 0.0 || s >= w || t < 0.0 || t >= h) {
                return Spectrum(0.0);
            }
        }
        return pyramid_[level](s, t);
    }

}  // namespace spica
