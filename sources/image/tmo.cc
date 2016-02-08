#define SPICA_API_EXPORT
#include "tmo.h"

#include <cmath>
#include <vector>
#include <algorithm>

#include "image.h"
#include "birateral.h"

namespace spica {

    namespace {
    
        double logMean(const Image& image) {
            const int width = image.width();
            const int height = image.height();
            
            double ret = 0.0;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    ret += log(image(x, y).luminance() + EPS);                    
                }
            }

            return exp(ret / (width * height));
        }

        double lumMax(const Image& image) {
            const int width = image.width();
            const int height = image.height();

            double ret = 0.0;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    ret = std::max(ret, image(x, y).luminance());
                }
            }

            return ret;
        }

    }  // anonymous namespace

    GammaTmo::GammaTmo(double gamma)
        : Tmo{}
        , _gamma{gamma} {
    }

    GammaTmo::~GammaTmo() {
    }

    Image GammaTmo::apply(const spica::Image& image) const {
        Assertion(_gamma >= EPS, "Too small gamma is specified!!");

        const int width  = image.width();
        const int height = image.height();

        const double invG = 1.0 / _gamma;
        Image ret(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const Color& color = image(x, y);
                const double r = clamp(pow(color.red(), invG), 0.0, 1.0);
                const double g = clamp(pow(color.green(), invG), 0.0, 1.0);
                const double b = clamp(pow(color.blue(), invG), 0.0, 1.0);
                ret.pixel(x, y) = Color(r, g, b);
            }
        }
        return std::move(ret);
    }

    ReinhardTmo::ReinhardTmo(double alpha)
        : Tmo{}
        , _alpha{alpha} {
    }

    ReinhardTmo::~ReinhardTmo() {
    }

    Image ReinhardTmo::apply(const spica::Image& image) const {
        const int width = image.width();
        const int height = image.height();

        const double Lwa = logMean(image);
        const double Lwhite = whitePoint(image);
        const double Lwhite2 = Lwhite * Lwhite;
        
        Image ret(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double L = image(x, y).luminance();
                const double Lscaled = (_alpha * L) / Lwa;
                const double Ld = (Lscaled * (1.0 + Lscaled / Lwhite2)) / (1.0 + Lscaled);

                ret.pixel(x, y) = image(x, y) / (L + EPS) * Ld;
            }
        }

        return std::move(ret);
    }

    double ReinhardTmo::whitePoint(const spica::Image& image) {
        const int width  = image.width();
        const int height = image.height();
        const int n = width * height;

        std::vector<double> Ls(width * height);
        int cnt = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Ls[cnt++] = image(x, y).luminance();            
            }
        }

        std::sort(Ls.begin(), Ls.end());

        const double Lmax = Ls[std::min((int)(n * 0.99), n - 1)];
        const double Lmin = Ls[std::min((int)(n * 0.01), n - 1)];
        const double log2Max = log2(Lmax + EPS);
        const double log2Min = log2(Lmin + EPS);

        return 1.5 * pow(2.0, log2Max - log2Min - 5);
    }

    DragoTmo::DragoTmo(double Ldmax, double p)
        : Tmo{}
        , _Ldmax{Ldmax}
        , _p{p} {
    }

    DragoTmo::~DragoTmo() {
    }

    Image DragoTmo::apply(const Image& image) const {
        const int width = image.width();
        const int height = image.height();

        const double Lwa = logMean(image) / (pow(1.0 + _p - 0.85, 5.0));
        const double Lmax = lumMax(image);

        const double Lmax_wa = Lmax / Lwa;

        const double c1 = log(_p) / log(0.5);
        const double c2 = (_Ldmax / 100.0) / log10(1.0 + Lmax_wa);

        Image ret(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double L = image(x, y).luminance();
                const double L_wa = L / Lwa;
                const double Ld = c2 * log(1.0 + L_wa) / log(2.0 + 8.0 * pow(L_wa / Lmax_wa, c1));
            
                ret.pixel(x, y) = image(x, y) / (L + EPS) * Ld;
            }
        }

        return std::move(ret);
    }

    DurandTMO::DurandTMO(double sigmaSpace, double sigmaColor, double targetContrast)
        : Tmo{}
        , _sigmaSpace{sigmaSpace}
        , _sigmaColor{sigmaColor}
        , _targetContrast{targetContrast} {
    }

    DurandTMO::~DurandTMO() {
    }

    Image DurandTMO::apply(const Image& image) const {
        const int width  = image.width();
        const int height = image.height();

        Image L(width, height);
        Image tmp(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double l = image(x, y).luminance();
                L.pixel(x, y) = Color(l, l, l);
                tmp.pixel(x, y) = image(x, y) / (l + EPS);
            }
        }

        Image Lbase, Ldetail;
        birateralSeparation(L, _sigmaSpace, _sigmaColor, &Lbase, &Ldetail);

        double maxLogBase = -INFTY;
        double minLogBase =  INFTY;
        Image logBase(width, height);
        Image logDetail(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double l = log10(Lbase(x, y).luminance() + EPS);
                maxLogBase = std::max(maxLogBase, l);
                minLogBase = std::min(minLogBase, l);
                logBase.pixel(x, y) = Color(l, l, l);

                const double d = log10(Ldetail(x, y).luminance() + EPS);
                logDetail.pixel(x, y) = Color(d, d, d);
            }
        }

        const double compressionFactor = log(_targetContrast) / (maxLogBase - minLogBase);
        const double logAbsolute = compressionFactor * maxLogBase;

        Image ret(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double logCompressed = logBase(x, y).luminance() * compressionFactor + logDetail(x, y).luminance() - logAbsolute;
                const Color compressed = tmp(x, y) * pow(10.0, logCompressed);
                ret.pixel(x, y) = compressed.clamp(Color(0.0, 0.0, 0.0), Color(1.0, 1.0, 1.0));                
            }
        }

        return std::move(ret);
    }

    void DurandTMO::birateralSeparation(const Image& L, double sigma_s, double sigma_r, Image* Lbase, Image* Ldetail) const {
        const int width  = L.width();
        const int height = L.height();
        
        Image logL(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double l = log10(L(x, y).luminance() + 1.0e-6);
                logL.pixel(x, y) = Color(l, l, l);
            }
        }

        Image filL;
        sigma_s = std::max(width, height) * sigma_s;
        birateral(logL, &filL, sigma_s, sigma_r);

        Lbase->resize(width, height);
        Ldetail->resize(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const double l = std::max(0.0, pow(10.0, filL(x, y).luminance()) - 1.0e-6);
                Lbase->pixel(x, y) = Color(l, l, l);
                Ldetail->pixel(x, y) = L(x, y) / (l + EPS);
            }
        }        
    }



}  // namespace spica
