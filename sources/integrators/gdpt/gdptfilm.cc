#include "gdptfilm.h"

#include "core/film.h"

#ifdef SPICA_WITH_FFTW
#include <fftw3.h>
#endif

namespace spica {

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

void GDPTFilm::save(int i, const std::string &solver) const {
    Image result;
    if (solver == "L1") {
        result = solveL1();
    } else if (solver == "L2") {
        result = solveL2();
    } else if (solver == "fourier") {
        result = solveFourier();
    } else {
        FatalError("Unknown solver: %s", solver.c_str());
    }
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

    // Weights
    std::vector<std::vector<double>> coarseWeights(width, std::vector<double>(height, 1.0));
    std::vector<std::vector<double>> dxWeights(width, std::vector<double>(height, 1.0));
    std::vector<std::vector<double>> dyWeights(width, std::vector<double>(height, 1.0));

    // Solve Poisson with Gauss-Seidel
    static const int maxIter = 100;
    static const int updateInterval = 10;
    double alpha = 0.2;

    const auto clampX = [&](int x) -> int { return std::max(0, std::min(x, width  - 1)); };
    const auto clampY = [&](int y) -> int { return std::max(0, std::min(y, height - 1)); };

    Image output = coarse;
    for (int it = 0; it < maxIter; it++) {    
        // Perform Gauss-Seidel step
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double wc = coarseWeights[x][y];
                const double wx1 = dxWeights[x][y];
                const double wx2 = dxWeights[clampX(x + 1)][y];
                const double wy1 = dyWeights[x][y];
                const double wy2 = dyWeights[x][clampY(y + 1)];

                output.pixel(x, y) = (1.0 / (alpha * alpha * wc + wx1 + wx2 + wy1 + wy2)) * (
                    alpha * alpha * wc * coarse(x, y) +
                    wx1 * (output(clampX(x - 1), y) + gradX(x, y)) +
                    wx2 * (output(clampX(x + 1), y) - gradX(clampX(x + 1), y)) +
                    wy1 * (output(x, clampY(y - 1)) + gradY(x, y)) +
                    wy2 * (output(x, clampY(y + 1)) - gradY(x, clampY(y + 1))));
            }
        }

        if (it == maxIter - 1) break;

        // Update weights
        if ((it + 1) % updateInterval == 0) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    coarseWeights[x][y] = 1.0 / std::max(EPS, std::abs((coarse(x, y) - output(x, y)).luminance()));
                    const Spectrum gx = output(x, y) - output(clampX(x - 1), y);
                    dxWeights[x][y] = 1.0 / std::max(EPS, std::abs((gradX(x, y) - gx).luminance()));
                    const Spectrum gy = output(x, y) - output(x, clampY(y - 1));
                    dyWeights[x][y] = 1.0 / std::max(EPS, std::abs((gradY(x, y) - gy).luminance()));
                }
            }    
        }
    }

    // Save gradient
    #ifdef GDPT_TAKE_LOG
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
    #endif

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
    #ifdef GDPT_TAKE_LOG
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
    #endif

    return std::move(output);
}

Image GDPTFilm::solveFourier() const {
    #ifndef SPICA_WITH_FFTW
    FatalError("Method \"solveFourier\" requires FFTW library!");
    #else
    {
        static const double dataCost = 0.2;
    
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
        
        // Copy image data
        auto imgData = std::make_unique<double[]>(width * height * 3);
        auto imgGradX = std::make_unique<double[]>(width * height * 3);
        auto imgGradY = std::make_unique<double[]>(width * height * 3);
        auto imgOut = std::make_unique<double[]>(width * height * 3);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                for (int c = 0; c < 3; c++) {
                    const int index = (y * width + x) * 3 + c;
                    imgData[index] = coarse(x, y)[c];
                    imgGradX[index] = gradX(x, y)[c];
                    imgGradY[index] = gradY(x, y)[c];
                }
            }
        }

        const int nodeCount = width * height;
        double *fftBuff = (double*)fftw_malloc(sizeof(*fftBuff) * nodeCount);
        //compute two 1D lookup tables for computing the DCT of a 2D Laplacian on the fly
        double *ftLapY = (double*)fftw_malloc(sizeof(*ftLapY) * height);
        double *ftLapX = (double*)fftw_malloc(sizeof(*ftLapX) * width);
        for(int x = 0; x < width; x++) {
            ftLapX[x] = 2.0 * std::cos(PI * x / (width - 1));
        }
        for(int y = 0; y < height; y++) {
            ftLapY[y] = -4.0 + (2.0 * std::cos(PI * y / (height - 1)));
        }
        //Create a DCT-I plan for, which is its own inverse.
        fftw_plan fftPlan;
        fftPlan = fftw_plan_r2r_2d(height, width,
                                   fftBuff, fftBuff,
                                   FFTW_REDFT00, FFTW_REDFT00, FFTW_ESTIMATE); //use FFTW_PATIENT when plan can be reused
        for(int iChannel = 0; iChannel < 3; iChannel++) {
            int nodeAddr        = 0;
            int pixelAddr       = iChannel;
            int rightPixelAddr  = 3 + iChannel;
            int topPixelAddr    = (width * 3) + iChannel;
            double dcSum = 0.0;
            
            // compute h_hat from u, gx, gy (see equation 48 in Bhat's paper), as well as the DC term of u's DCT.
            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width;  x++,
                    nodeAddr++, pixelAddr += 3, rightPixelAddr += 3, topPixelAddr += 3) {
                    // Compute DC term of u's DCT without computing the whole DCT.
                    double dcMult = 1.0;
                    if((x > 0) && (x < width  - 1))
                        dcMult *= 2.0;
                    if((y > 0) && (y < height - 1))
                        dcMult *= 2.0;
                    dcSum += dcMult * imgData[pixelAddr];
                    
                    fftBuff[nodeAddr] = dataCost * imgData[pixelAddr];
                    
                    // Subtract g^x_x and g^y_y, with boundary factor of -2.0 to account for boundary reflections implicit in the DCT
                    if((x > 0) && (x < width - 1))
                        fftBuff[nodeAddr] -= (imgGradX[rightPixelAddr] - imgGradX[pixelAddr]);
                    else
                        fftBuff[nodeAddr] -= (-2.0 * imgGradX[pixelAddr]);
                    
                    if((y > 0) && (y < height - 1))
                        fftBuff[nodeAddr] -= (imgGradY[topPixelAddr] - imgGradY[pixelAddr]);
                    else
                        fftBuff[nodeAddr] -= (-2.0 * imgGradY[pixelAddr]);
                }
            }
            //transform h_hat to H_hat by taking the DCT of h_hat
            fftw_execute(fftPlan);
            
            //compute F_hat using H_hat (see equation 29 in Bhat's paper)
            nodeAddr = 0;
            for(int y = 0; y < height; y++)
                for(int x = 0; x < width;  x++, nodeAddr++) {
                    float ftLapResponse = ftLapY[y] + ftLapX[x];
                    fftBuff[nodeAddr] /= (dataCost - ftLapResponse);
                }
            /* Set the DC term of the solution to the value computed above (i.e., the DC term of imgData).
             * set dcSum to the desired average when dataCost=0
             */
            fftBuff[0] = dcSum;
            
            //transform F_hat to f_hat by taking the inverse DCT of F_hat
            fftw_execute(fftPlan);
            double fftDenom = 4.0 * (width - 1) * (height - 1);
            pixelAddr = iChannel;
            for(int iNode = 0; iNode < nodeCount; iNode++, pixelAddr += 3) {
                imgOut[pixelAddr] = fftBuff[iNode] / fftDenom;
            }
        }
        
        fftw_free(fftBuff);
        fftw_free(ftLapX);
        fftw_free(ftLapY);
        fftw_destroy_plan(fftPlan);
        
        // Copy output
        Image output(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double r = imgOut[(y * width + x) * 3 + 0];
                const double g = imgOut[(y * width + x) * 3 + 1];
                const double b = imgOut[(y * width + x) * 3 + 2];
                output.pixel(x, y) = Spectrum(r, g, b);                
            }
        }

        // Save gradient
        #ifdef GDPT_TAKE_LOG
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
        #endif

        return std::move(output);
    }
    #endif
}

}  // namespace spica
