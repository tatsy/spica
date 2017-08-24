#include "gdptfilm.h"

#include "core/film.h"

namespace spica {

static const int offsetX[] = { -1, 1, 0, 0 };
static const int offsetY[] = { 0, 0, -1, 1 };

GDPTFilm::GDPTFilm(const std::shared_ptr<Film> &film)
    : film_{film} {
    // Initialize buffers
    const int width = film_->resolution().x();
    const int height = film_->resolution().y();
    image_ = Image(width, height);
    image_.fill(Spectrum(0.0));
    for (int k = 0; k < 4; k++) {
        gradients_[k].resize(width, height);
        gradients_[k].fill(Spectrum(0.0));
    }

    // Initialize weight buffers
    weights_.assign(width, std::vector<double>(height, 0.0));
    gradWeights_.resize(4);
    for (int k = 0; k < 4; k++) {
        gradWeights_[k].assign(width, std::vector<double>(height, 0.0));
    }
}

void GDPTFilm::save(int i) const {
    const Image result = solveL2();
    film_->setImage(result);
    film_->save(i);
}

void GDPTFilm::addPixel(int x, int y, const Spectrum &color, double weight) {
    image_.pixel(x, y) += weight * color;
    weights_[x][y] += weight;
}

void GDPTFilm::addGradient(int x, int y, int index, const Spectrum &grad, double weight) {
    gradients_[index].pixel(x, y) += weight * grad;
    gradWeights_[index][x][y] += weight;
}

double GDPTFilm::evalFilter(const Point2d &pixel) const {
    return film_->weight(pixel);
}

Image GDPTFilm::evalImage() const {
    Image res = image_;
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            res.pixel(x, y) /= (weights_[x][y] + EPS);
        }
    }
    return std::move(res);
}

Image GDPTFilm::evalGrad(int index) const {
    Image res = gradients_[index];
    for (int y = 0; y < image_.height(); y++) {
        for (int x = 0; x < image_.width(); x++) {
            res.pixel(x, y) /= (gradWeights_[index][x][y] + EPS);
        }
    }
    return std::move(res);
}

Image GDPTFilm::solveL1() const {
    // Evaluate coarse image
    Image coarse = evalImage();
    const int width = coarse.width();
    const int height = coarse.height();

    // Evaluate gradients
    std::array<Image, 4> grads;
    for (int k = 0; k < 4; k++) {
        grads[k] = evalGrad(k);
    }

    // Copy forward difference
    Image gradX(width, height);
    Image gradY(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (x == 0) {
                gradX.pixel(x, y) = grads[0](x, y);
            }

            if (y == 0) {
                gradY.pixel(x, y) = grads[3](x, y);
            }
        }
    }

    // Solve Poisson with Gauss-Seidel
    static const int maxIter = 50;
    static const double alpha = 0.2;

    const auto clampX = [&](int x) -> int { return std::max(0, std::min(x, width - 1)); };
    const auto clampY = [&](int y) -> int { return std::max(0, std::min(y, height - 1)); };

    Image output = coarse;
    for (int it = 0; it < maxIter; it++) {    
        // Perform Gauss-Seidel step
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                output.pixel(x, y) = (1.0 / (4.0 + alpha * alpha)) * (
                    alpha * alpha * coarse(x, y) +
                    output(clampX(x - 1), y) + output(clampX(x + 1), y) + output(x, clampY(y - 1)) + output(x, clampY(y + 1)) +
                    gradX(x, y) - gradX(clampX(x + 1), y) + gradY(x, y) - gradY(x, clampY(y + 1)));            
            }
        }
    }

    return std::move(output);
}

Image GDPTFilm::solveL2() const {
    // Evaluate coarse image
    Image coarse = evalImage();
    const int width = coarse.width();
    const int height = coarse.height();

    // Evaluate gradients
    std::array<Image, 4> grads;
    for (int k = 0; k < 4; k++) {
        grads[k] = evalGrad(k);
    }

    // Copy forward difference
    Image gradX(width, height);
    Image gradY(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (x == 0) {
                gradX.pixel(x, y) = grads[0](x, y);
            } else {
                gradX.pixel(x, y) = (grads[0](x, y) + grads[1](x - 1, y));
            }

            if (y == 0) {
                gradY.pixel(x, y) = grads[2](x, y);
            } else {
                gradY.pixel(x, y) = (grads[2](x, y) + grads[3](x, y - 1));
            }
        }
    }

    // Solve Poisson with Gauss-Seidel
    static const int maxIter = 100;
    double alpha = 0.2;

    const auto clampX = [&](int x) -> int { return std::max(0, std::min(x, width  - 1)); };
    const auto clampY = [&](int y) -> int { return std::max(0, std::min(y, height - 1)); };

    Image output = coarse;
    for (int it = 0; it < maxIter; it++) {    
        // Perform Gauss-Seidel step
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                output.pixel(x, y) = (1.0 / (4.0 + alpha * alpha)) * (
                    alpha * alpha * coarse(x, y) +
                    output(clampX(x - 1), y) + output(clampX(x + 1), y) +
                    output(x, clampY(y - 1)) + output(x, clampY(y + 1)) +
                    gradX(x, y) - gradX(clampX(x + 1), y) +
                    gradY(x, y) - gradY(x, clampY(y + 1)));
            }
        }
    }

    // Save gradient
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Spectrum c;
            c = gradX(x, y);
            gradX.pixel(x, y) = Spectrum(std::abs(c.red()), std::abs(c.green()), std::abs(c.blue()));
            c = gradY(x, y);
            gradY.pixel(x, y) = Spectrum(std::abs(c.red()), std::abs(c.green()), std::abs(c.blue()));
        }
    }
    coarse.save("coarse.png");
    gradX.save("gradX.png");
    gradY.save("gradY.png");

    return std::move(output);
}

}  // namespace spica
