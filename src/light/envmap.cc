#define SPICA_API_EXPORT
#include "envmap.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "../core/common.h"
#include "../renderer/photon_map.h"

namespace spica {

    namespace {
        void directionToPolarCoord(const Vector3D& dir, double* theta, double* phi) {
            *theta = acos(dir.y());
            *phi   = atan2(dir.z(), dir.x());
            if (*phi < 0.0) {
                *phi += 2.0 * PI;
            }
        }
    }

    Envmap::Envmap()
        : ILight{LightType::Envmap}
        , _sphere{}
        , _image{}
        , _importance{}
        , _pdf{}
        , _cdf{} {
    }

    Envmap::Envmap(const Sphere& boundSphere, const std::string& filename)
        : ILight{LightType::Envmap}
        , _sphere{boundSphere}
        , _image{}
        , _importance{}
        , _pdf{}
        , _cdf{} {
        _image.load(filename);
        createImportanceMap();
    }

    Envmap::Envmap(const Sphere& boundSphere, const Image& image)
        : ILight{LightType::Envmap}
        , _sphere{boundSphere}
        , _image{image}
        , _importance{}
        , _pdf{}
        , _cdf{} {
        createImportanceMap();
    }

    Envmap::Envmap(const Envmap& envmap)
        : Envmap{} {
        this->operator=(envmap);
    }

    Envmap::Envmap(Envmap&& envmap)
        : Envmap{} {
        this->operator=(std::move(envmap));
    }

    Envmap::~Envmap() {
    }

    Envmap& Envmap::operator=(const Envmap& envmap) {
        ILight::operator=(envmap);
        _sphere = envmap._sphere;
        _image  = envmap._image;
        _importance = envmap._importance;
        _pdf = envmap._pdf;
        _cdf = envmap._cdf;
        return *this;
    }

    Envmap& Envmap::operator=(Envmap&& envmap) {
        ILight::operator=(std::move(envmap));
        _sphere = envmap._sphere;
        _image  = std::move(envmap._image);
        _importance = std::move(envmap._importance);
        _pdf = std::move(envmap._pdf);
        _cdf = std::move(envmap._cdf);
        return *this;
    }

    void Envmap::resize(int width, int height) {
        _image.resize(width, height);
        _importance.resize(width, height);
    }

    void Envmap::clearColor(const Color& color) {
        _image.fill(color);
    }

    Color Envmap::sampleFromDir(const Vector3D& dir) const {
        double theta, phi;
        directionToPolarCoord(dir, &theta, &phi);
        
        const double iblu = phi   / (2.0 * PI);
        const double iblv = theta / PI;

        const double iblx = clamp(iblu * _image.width(),  0.0, _image.width()  - 1.0);
        const double ibly = clamp(iblv * _image.height(), 0.0, _image.height() - 1.0);

        return _image(iblx, ibly);
    }

    Color Envmap::directLight(const Vector3D& dir) const {
        return sampleFromDir(dir);
    }

    double Envmap::area() const {
        return _sphere.area();
    }

    LightSample Envmap::sample(double r1, double r2, double r3) const {
        const int width  = IMPORTANCE_MAP_SIZE;
        const int height = IMPORTANCE_MAP_SIZE;

        const int index = std::lower_bound(_cdf.begin(), _cdf.end(), r1) - _cdf.begin();

        // Direction
        const int ix = index % width;
        const int iy = index / width;
        const double u = (ix + r2) / width;
        const double v = (iy + r3) / height;

        const double phi = u * 2.0 * PI;
        const double y = (1.0 - v) * 2.0 - 1.0;
        const Vector3D dir = Vector3D(sqrt(1.0 - y * y) * cos(phi), y, sqrt(1.0 - y * y) * sin(phi));
    
        Vector3D pos = _sphere.center() + dir * _sphere.radius();
        Vector3D normal = -dir;

        const double A = _sphere.area();
        Color Le = sampleFromDir(dir) * PI / (A * _pdf[index]);
        return LightSample(pos, normal, Le, A / (width * height));
    }

    void Envmap::createImportanceMap() {
        _importance.resize(IMPORTANCE_MAP_SIZE, IMPORTANCE_MAP_SIZE);

        const int width  = _importance.width();
        const int height = _importance.height();

        double total = 0.0;
        for (int iy = 0; iy < height; iy++) {
            for (int ix = 0; ix < width; ix++) {
                const double u = static_cast<double>(ix) / width;
                const double v = static_cast<double>(iy) / height;
                const double next_u = static_cast<double>(ix + 1) / width;
                const double next_v = static_cast<double>(iy + 1) / height;

                const int begin_x = static_cast<int>(u * _image.width());
                const int end_x = clamp<int>(static_cast<int>(next_u * _image.width()), 0, _image.width());
                const int begin_y = static_cast<int>(v * _image.height());
                const int end_y = clamp<int>(static_cast<int>(next_v * _image.height()), 0, _image.height());

                Color accum;
                const int area = (end_y - begin_y) * (end_x - begin_x);
                for (int ty = begin_y; ty < end_y; ty++) {
                    for (int tx = begin_x; tx < end_x; tx++) {
                        accum += _image(tx, ty);
                    }
                }

                _importance.pixel(ix, iy) = Color(1.0, 1.0, 1.0) * accum.luminance() / area;
                total += _importance(ix, iy).red();
            }
        }

        // Normalization
        for (int iy = 0; iy < height; iy++) {
            for (int ix = 0; ix < width; ix++) {
                _importance.pixel(ix, iy) = _importance(ix, iy) / (total + EPS);
            }
        }

        // Warped importance map
        total = 0.0;
        _pdf.resize(width * height);
        const int superX = _image.width() / width;
        const int superY = _image.height() / height;
        for (int iy = 0; iy < height; iy++) {
            for (int ix = 0; ix < width; ix++) {
                Color accum;
                for (int sy = 0; sy < superY; sy++) {
                    for (int sx = 0; sx < superX; sx++) {
                        const double u = (ix + static_cast<double>(sx) / superX) / width;
                        const double v = (iy + static_cast<double>(sy) / superY) / height;

                        const double phi = u * 2.0 * PI;
                        const double y = (1.0 - v) * 2.0 - 1.0;

                        const Vector3D dir = Vector3D(sqrt(1.0 - y * y) * cos(phi), y, sqrt(1.0 - y * y) * sin(phi));
                        accum += sampleFromDir(dir) / (superX * superY);
                    }
                }

                _pdf[iy * width + ix] = accum.luminance();
                total += _pdf[iy * width + ix];
            }
        }

        // Normalization
        _cdf.resize(width * height);
        _pdf[0] /= (total + EPS);
        _cdf[0] = _pdf[0];
        for (int i = 1; i < width * height; i++) {
            _pdf[i] /= (total + EPS);
            _cdf[i] = _pdf[i] + _cdf[i - 1];
        }
    }

    ILight* Envmap::clone() const {
        return new Envmap(*this);
    }

    const Image& Envmap::getImage() const {
        return _image;
    }

}  // namespace spica


