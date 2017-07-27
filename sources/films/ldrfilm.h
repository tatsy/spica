#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LDRFILM_H_
#define _SPICA_LDRFILM_H_

#include "core/common.h"
#include "core/film.h"

namespace spica {

class SPICA_EXPORTS LDRFilm : public Film {
public:
    LDRFilm(const Point2i& resolution,
            const std::shared_ptr<Filter> &filter,
            const std::string& filename = "image",
            const std::shared_ptr<std::function<void(const Image&)>> &callback = nullptr);

    LDRFilm(const RenderParams &params);

protected:
    void saveImage(const std::string &filename, const Image &image) const override;
};

SPICA_EXPORT_PLUGIN(LDRFilm, "Low dynamic range film");

}  // namespace spica



#endif  // _SPICA_LDRFILM_H_
