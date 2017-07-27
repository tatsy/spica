#define SPICA_API_EXPORT
#include "ldrfilm.h"

#include <core/filter.h>

namespace spica {

LDRFilm::LDRFilm(const Point2i& resolution,
                 const std::shared_ptr<Filter> &filter,
                 const std::string& filename,
                 const std::shared_ptr<std::function<void(const Image&)>> &callback)
    : Film{resolution, filter, filename, callback} {
}

LDRFilm::LDRFilm(const RenderParams &params) 
    : LDRFilm{Point2i(params.getInt("width"), params.getInt("height")),
              std::static_pointer_cast<Filter>(params.getObject("rfilter"))} {
}

void LDRFilm::saveImage(const std::string &filename, const Image &image) const {
    image.save(filename + ".png");
}

}  // namespace spica