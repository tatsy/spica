#define SPICA_API_EXPORT
#include "film.h"

#include "../core/path.h"
#include "tmo.h"

namespace spica {

Film::Film(const Point2i& resolution, std::unique_ptr<Filter>&& filter,
           const std::string& filename,
           std::unique_ptr<std::function<void(const Image&)>>&& callback)
    : resolution_{ resolution }
    , filter_{ std::move(filter) }
    , filename_{ filename }
    , image_{ (unsigned int)resolution.x(),
              (unsigned int)resolution.y() }
    , weights_{}
    , saveCallback_{ std::move(callback) } {
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

    std::string ext = path::getExtension(filename_);

    char savefile[512];
    const char* format = filename_.c_str();
    sprintf(savefile, format, id);

    if (ext != ".hdr") {
        GammaTmo tmo(2.2);
        res = tmo.apply(res);
    }
    res.save(savefile);

    MsgInfo("save: %s", savefile);
    if (saveCallback_) {
        (*saveCallback_)(res);
    }
}

void Film::setImage(const Image& image) {
    this->image_ = image;
    weights_.assign(resolution_.x(), std::vector<double>(resolution_.y(), 1.0));
}

void Film::addPixel(const Point2i& pixel, const Point2d& pInPixel, 
                    const Spectrum& color) {
    const double dx = pInPixel.x() - 0.5;
    const double dy = pInPixel.y() - 0.5;
    const double weight = filter_->evaluate(Point2d(dx, dy));

    image_.pixel(pixel.x(), pixel.y()) += weight * color;
    weights_[pixel.x()][pixel.y()]     += weight;
}

}  // namespace spica
