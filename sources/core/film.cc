#define SPICA_API_EXPORT
#include "film.h"

#include "core/path.h"
#include "core/tmo.h"

namespace spica {

Film::Film(const Point2i& resolution,
           const std::shared_ptr<Filter> &filter,
           const std::string& filename,
           const std::shared_ptr<std::function<void(const Image&)>> &callback)
    : resolution_{ resolution }
    , filter_{ filter }
    , filename_{ filename }
    , image_{ (unsigned int)resolution.x(),
              (unsigned int)resolution.y() }
    , weights_{}
    , saveCallback_{ callback } {
    image_.fill(RGBSpectrum(0.0, 0.0, 0.0));
    weights_.assign(resolution_.x(), std::vector<double>(resolution_.y(), 0.0));
    samples_.assign(resolution_.x(), std::vector<int>(resolution_.y(), 0));
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

    saveImage(savefile, res);

    if (saveCallback_) {
        (*saveCallback_)(res);
    }
}

void Film::saveMLT(double scale, int id) const {
    Image res = image_;
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            res.pixel(x, y) *= scale * samples_[x][y] / (weights_[x][y] + EPS);
        }
    }

    char savefile[512];
    const char* format = filename_.c_str();
    sprintf(savefile, format, id);
    saveImage(savefile, res);

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
    samples_[pixel.x()][pixel.y()]     += 1;
}

void Film::addPixel(const Point2d& pixel, const Spectrum& color) {
    Point2i p((int)pixel.x(), (int)pixel.y());
    Point2d pd(pixel.x() - p.x(), pixel.y() - p.y());
    addPixel(p, pd, color);
}

}  // namespace spica
