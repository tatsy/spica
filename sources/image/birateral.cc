#define SPICA_API_EXPORT
#include "birateral.h"

#include <vector>

#include "../core/parallel.h"
#include "image.h"

namespace spica {

    namespace {

        // Recursive filter for vertical direction
        void recursiveFilterVertical(Image* out, const std::vector<std::vector<double> >& dct, double sigma_H) {
            const int width  = out->width();
            const int height = out->height();
            const int dim    = 3;
            const double a = exp(-sqrt(2.0) / sigma_H);

            std::vector<std::vector<double> > V = dct;
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height - 1; y++) {
                    V[y][x] = pow(a, V[y][x]);
                }
            }

            // if openmp is available, compute in parallel
            parallel_for (0, width, [&](int x) {
                for (int y = 1; y < height; y++) {
                    double p = V[y - 1][x];
                    for (int c = 0; c < dim; c++) {
                        const double val1 = (*out)(x, y)[c];
                        const double val2 = (*out)(x, y - 1)[c];
                        out->pixel(x, y).ref(c) = val1 + p * (val2 - val1);
                    }
                }

                for (int y = height - 2; y >= 0; y--) {
                    const double p = V[y][x];
                    for (int c = 0; c < dim; c++) {
                        const double val1 = (*out)(x, y)[c];
                        const double val2 = (*out)(x, y + 1)[c];
                        out->pixel(x, y).ref(c) = val1 + p * (val2 - val1);
                    }
                }
            });
        }

        // Recursive filter for horizontal direction
        void recursiveFilterHorizontal(Image* out, const std::vector<std::vector<double> >& dct, double sigma_H) {
            const int width  = out->width();
            const int height = out->height();
            const int dim    = 3;
            const double a = exp(-sqrt(2.0) / sigma_H);

            std::vector<std::vector<double> > V = dct;
            for (int x = 0; x < width - 1; x++) {
                for (int y = 0; y < height; y++) {
                    V[y][x] = pow(a, V[y][x]);
                }
            }

            // if openmp is available, compute in parallel
            parallel_for (0, height, [&](int y) {
                for (int x = 1; x < width; x++) {
                    const double p = V[y][x - 1];
                    for (int c = 0; c < dim; c++) {
                        const double val1 = (*out)(x, y)[c];
                        const double val2 = (*out)(x - 1, y)[c];
                        out->pixel(x, y).ref(c) = val1 + p * (val2 - val1);
                    }
                }

                for (int x = width - 2; x >= 0; x--) {
                    const double p = V[y][x];
                    for (int c = 0; c < dim; c++) {
                        const double val1 = (*out)(x, y)[c];
                        const double val2 = (*out)(x + 1, y)[c];
                        out->pixel(x, y).ref(c) = val1 + p * (val2 - val1);
                    }
                }
            });
        }

        // Domain transform filtering
        void domainTransformFilter(const Image& img, Image* out, double sigma_s, double sigma_r, int maxiter) {
            const int width  = img.width();
            const int height = img.height();
            const int dim    = 1;

            // compute derivatives of transformed domain "dct"
            // and a = exp(-sqrt(2) / sigma_H) to the power of "dct"
            std::vector<std::vector<double> > dctx(height, std::vector<double>(width - 1, 0.0));
            std::vector<std::vector<double> > dcty(height - 1, std::vector<double>(width, 0.0));
            const double ratio = sigma_s / sigma_r;

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width - 1; x++) {
                    double accum = 0.0;
                    for (int c = 0; c < dim; c++) {
                        accum += std::abs(img(x + 1, y)[c] - img(x, y)[c]);
                    }
                    dctx[y][x] = 1.0 + ratio * accum;
                }
            }

            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height - 1; y++) {
                    double accum = 0.0;
                    for (int c = 0; c < dim; c++) {
                        accum += std::abs(img(x, y + 1)[c] - img(x, y)[c]);
                    }
                    dcty[y][x] = 1.0 + ratio * accum;
                }
            }

            // Apply recursive folter maxiter times
            (*out) = img;
            for (int i = 0; i < maxiter; i++) {
                const double sigma_H = sigma_s * sqrt(3.0) * pow(2.0, maxiter - i - 1) / sqrt(pow(4.0, maxiter) - 1.0);
                recursiveFilterHorizontal(out, dctx, sigma_H);
                recursiveFilterVertical(out, dcty, sigma_H);
            }
        }

    }  // anonymous namespace

    void birateral(const Image& src, Image* dst, double sigma_s, double sigma_r) {
        domainTransformFilter(src, dst, sigma_s, sigma_r, 20);
    }

}  // namespace spica
