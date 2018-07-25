#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_GDPT_FILM_H_
#define _SPICA_GDPT_FILM_H_

#include <vector>
#include <memory>

#include "core/core.hpp"
#include "core/image.h"
#include "core/film.h"

namespace spica {

class GDPTFilm {
public:
    GDPTFilm(const std::shared_ptr<Film> &film, bool isDebug = false);

    void save(int i, const std::string &solver) const;
    void addPixel(int x, int y, const Spectrum &color, double weight);
    void addGradient(int x, int y, int index, const Spectrum &grad, double weight);
    double evalFilter(const Point2d &pixel) const;

private:
    Image evalImage() const;
    Image evalGrad(int index) const;
    Image solveL1() const;
    Image solveL2() const;
    Image solveFourier() const;
    void saveDebugImages(const Image &coarse, Image &gradX, Image &gradY) const;

    std::shared_ptr<Film> film_;
    bool isDebug_;

    Image image_;
    std::string saveDir_;
    std::vector<std::vector<double>> weights_;
    std::vector<std::vector<std::vector<double>>> gradWeights_;
    std::array<Image, 4> gradients_;
};

}  // namespace spica

#endif  // _SPICA_GDPT_FILM_H_
