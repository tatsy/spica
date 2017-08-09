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
            const std::string& filename,
            double gamma = 2.2,
            const std::shared_ptr<std::function<void(const Image&)>> &callback = nullptr);

    LDRFilm(RenderParams &params);

protected:
    void saveImage(const std::string &filename, const Image &image) const override;

private:
    double gamma_;
};

SPICA_EXPORT_PLUGIN(LDRFilm, "Low dynamic range film");

}  // namespace spica



#endif  // _SPICA_LDRFILM_H_
