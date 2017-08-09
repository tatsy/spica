#define SPICA_API_EXPORT
#include "ldrfilm.h"

#include <core/filter.h>
#include <core/tmo.h>

namespace spica {

LDRFilm::LDRFilm(const Point2i& resolution,
                 const std::shared_ptr<Filter> &filter,
                 const std::string& filename,
                 double gamma,
                 const std::shared_ptr<std::function<void(const Image&)>> &callback)
    : Film{resolution, filter, filename, callback}
    , gamma_{gamma} {
}

LDRFilm::LDRFilm(RenderParams &params) 
    : LDRFilm{Point2i(params.getInt("width", true), params.getInt("height", true)),
              std::static_pointer_cast<Filter>(params.getObject("rfilter", true)),
              params.getString("outputFile", "image"),
              params.getDouble("gamma", 2.2)} {
}

void LDRFilm::saveImage(const std::string &filename, const Image &image) const {
    const std::string outfile = filename + ".png";

    // Gamma correction and save
    GammaTmo tmo{gamma_};
    tmo.apply(image).save(outfile);

    MsgInfo("Save: %s", outfile.c_str());
}

}  // namespace spica