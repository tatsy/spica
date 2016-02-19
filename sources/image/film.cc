#define SPICA_API_EXPORT
#include "film.h"

#include "tmo.h"

namespace spica {

Film::Film()
    : resolution_{}
    , filter_{ nullptr }
    , filename_{}
    , image_{} {
}

Film::Film(const Point2i& resolution, std::unique_ptr<Filter>& filter,
           const std::string& filename)
    : resolution_{ resolution }
    , filter_{ filter.get() }
    , filename_{ filename }
    , image_{ resolution.x(), resolution.y() }
    , weights_{} {
    image_.fill(RGBSpectrum(0.0, 0.0, 0.0));
    weights_.assign(resolution_.x(), std::vector<double>(resolution_.y(), 0.0));
}

void Film::save(int id) const {
    Image res = image_;
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            res.pixel(x, y) /= (weights_[x][y] + EPS);
        }
    }

    char savefile[512];
    const char* format = filename_.c_str();
    sprintf(savefile, format, id);
    
    GammaTmo tmo(2.2);
    res = tmo.apply(res);
    res.save(savefile);

    MsgInfo("save: %s", savefile);
}

void Film::addPixel(const Point2i& pixel, const Point2D& pInPixel, 
                    const Spectrum& color) {
    const double dx = pInPixel.x() - 0.5;
    const double dy = pInPixel.y() - 0.5;
    const double weight = filter_->evaluate(Point2D(dx, dy));

    image_.pixel(pixel.x(), pixel.y()) += weight * color;
    weights_[pixel.x()][pixel.y()]     += weight;
}

}  // namespace spica
