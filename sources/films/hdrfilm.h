#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_HDRFILM_H_
#define _SPICA_HDRFILM_H_

#include "core/common.h"
#include "core/film.h"
#include "core/renderparams.h"

namespace spica {

class SPICA_EXPORTS HDRFilm : public Film {
public:
    HDRFilm(const Point2i& resolution,
            const std::shared_ptr<Filter> &filter,
            const std::string& filename = "image",
            const std::shared_ptr<std::function<void(const Image&)>> &callback = nullptr);

    HDRFilm(RenderParams &params);

protected:
    void saveImage(const std::string &filename, const Image &image) const override;
};

SPICA_EXPORT_PLUGIN(HDRFilm, "High dynamic range film");

}  // namespace spica

#endif  // _SPICA_HDRFILM_H_
