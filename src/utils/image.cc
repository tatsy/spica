#define SPICA_IMAGE_EXPORT
#include "image.h"

#include <cmath>
#include <cstring>
#include <fstream>
#include <algorithm>

#include "common.h"

namespace spica {

    Image::Image()
        : _width(0)
        , _height(0)
        , _pixels(0)
    {
    }

    Image::Image(int width, int height)
        : _width(width)
        , _height(height)
        , _pixels(new Color[width * height])
    {
    }

    Image::Image(const Image& image) 
        : _width(image._width)
        , _height(image._height)
        , _pixels(new Color[image._width * image._height])
    {
        memcpy(_pixels, image._pixels, sizeof(Color) * image._width * image._height);
    }

    Image::~Image()
    {
        delete[] _pixels;
    }

    Image& Image::operator=(const Image& image) {
        delete[] _pixels;

        this->_width = image._width;
        this->_height = image._height;
        this->_pixels = new Color[image._width * image._height];
        memcpy(_pixels, image._pixels, sizeof(Color) * image._width * image._height);

        return *this;
    }

    const Color& Image::operator()(int x, int y) const {
		msg_assert(0 <= x && x < _width && 0 <= y && y < _height, "Pixel index out of bounds");
        return _pixels[y * _width + x];
    }

    Color& Image::pixel(int x, int y) {
		msg_assert(0 <= x && x < _width && 0 <= y && y < _height, "Pixel index out of bounds");
		return _pixels[y * _width + x];
    }

    void Image::savePPM(const std::string& filename) const {
        std::ofstream ofs(filename.c_str(), std::ios::out);
        ofs << "P3" << std::endl;
        ofs << _width << " " << _height << " 255" << std::endl;

        for (int i = 0; i < _width * _height; i++) {
            int r = toByte(_pixels[i].red());
            int g = toByte(_pixels[i].green());
            int b = toByte(_pixels[i].blue());
            ofs << r << " " << g << " " << b << std::endl;
        }
        ofs.close();
    }

    unsigned char Image::toByte(double d) {
        static const double invgamma = 1.0 / 2.2;
        d = std::max(0.0, std::min(d, 1.0));
        return (unsigned char)(255.0 * pow(d, invgamma));
    }
}
