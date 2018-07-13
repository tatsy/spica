#define SPICA_API_EXPORT
#include "hdrfilm.h"

#include <core/filter.h>

namespace spica {

HDRFilm::HDRFilm(const Point2i& resolution,
                 const std::shared_ptr<Filter> &filter,
                 const std::string& filename,
                 const std::shared_ptr<std::function<void(const Image&)>> &callback)
    : Film{resolution, filter, filename, callback} {
}

HDRFilm::HDRFilm(RenderParams &params)
    : HDRFilm{Point2i(params.getInt("width", true), params.getInt("height", true)),
              std::static_pointer_cast<Filter>(params.getObject("rfilter", true)),
              params.getString("outputFile")} {
}

void HDRFilm::saveImage(const std::string &filename, const Image &image) const {
    const std::string outfile = filename + ".hdr";
    image.save(outfile);
    MsgInfo("Save: %s", outfile.c_str());
}

}  // namespace spica