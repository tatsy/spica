#define SPICA_IMAGE_EXPORT
#include "image.h"

#if defined(_WIN32) || defined(__WIN32__)
#define PACKED(__declare__) __pragma(pack(push,1)) __declare__ __pragma(pack(pop)) 
#else
#define PACKED(__declare__) __declare__ __attribute__((__packed__))
#endif

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <algorithm>

#include "common.h"

namespace spica {

    namespace {
        PACKED(
        struct BitmapFileHeader {
            unsigned short bfType;
            unsigned int   bfSize;
            unsigned short bfReserved1;
            unsigned short bfReserved2;
            unsigned int   bfOffBits;
        });

        PACKED(
        struct BitmapCoreHeader {
            unsigned int   biSize;
            long biWidth;
            long biHeight;
            unsigned short biPlanes;
            unsigned short biBitCount;
            unsigned int   biCompression;
            unsigned int   biSizeImage;
            long biXPixPerMeter;
            long biYPixPerMeter;
            unsigned int  biClrUsed;
            unsigned int  biClrImportant;
        });

        PACKED(
        struct RGBTriple {
            unsigned char rgbBlue;
            unsigned char rgbGreen;
            unsigned char rgbRed;
        });
    }


    Image::Image()
        : _width(0)
        , _height(0)
        , _pixels(0)
    {
    }

    Image::Image(int width, int height)
        : _width(width)
        , _height(height)
        , _pixels(0)
    {
        msg_assert(width >= 0 && height >= 0, "Image size must be positive");
        _pixels = new Color[_width * _height];
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

    void Image::loadBMP(const std::string& filename) {
        BitmapFileHeader header;
        BitmapCoreHeader core;

        printf("ho");
        std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
        printf("ge\n");
        msg_assert(ifs.is_open(), "Failed to open file!!");

        ifs.read((char*)&header, sizeof(BitmapFileHeader));
        ifs.read((char*)&core, sizeof(BitmapCoreHeader));
        printf("header: %ld\n", sizeof(BitmapFileHeader));
        printf("  core: %ld\n", sizeof(BitmapCoreHeader));

        this->_width  = std::abs(core.biWidth);
        this->_height = std::abs(core.biHeight);
        this->_pixels = new Color[_width * _height];

        const int lineSize = (sizeof(RGBTriple) * _width + 3) / 4 * 4;
        char* lineBits = new char[lineSize];
        for (int y = 0; y < _height; y++) {
            ifs.read((char*)lineBits, lineSize);
            char* ptr = lineBits;
            for (int x = 0; x < _width; x++) {
                RGBTriple triple;
                memcpy(&triple, ptr, sizeof(RGBTriple));

                double red   = toReal(triple.rgbRed);
                double green = toReal(triple.rgbGreen);
                double blue  = toReal(triple.rgbBlue);
                this->_pixels[y * _width + x] = Color(red, green, blue);
                ptr += sizeof(RGBTriple);
            }
        }
        delete[] lineBits;

        ifs.close();
    }

    void Image::saveBMP(const std::string& filename) const {
        const int lineSize = (sizeof(RGBTriple) * _width + 3) / 4 * 4;
        const int totalSize = lineSize * _height;
        const int offBits = sizeof(BitmapFileHeader) + sizeof(BitmapCoreHeader);

        // Prepare file header
        BitmapFileHeader header;
        header.bfType = 'B' | ('M' << 8);
        header.bfSize = offBits + totalSize;
        header.bfReserved1 = 0;
        header.bfReserved2 = 0;
        header.bfOffBits = offBits;

        // Prepare core header
        BitmapCoreHeader core;
        core.biSize = 40;
        core.biWidth = _width;
        core.biHeight = -_height;
        core.biPlanes = 1;
        core.biBitCount = 24;
        core.biCompression = 0;
        core.biSizeImage = totalSize;
        core.biXPixPerMeter = 0;
        core.biYPixPerMeter = 0;
        core.biClrUsed = 0;
        core.biClrImportant = 0;

        std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::binary);
        ofs.write((char*)&header, sizeof(BitmapFileHeader));
        ofs.write((char*)&core, sizeof(BitmapCoreHeader));

        char* lineBits = new char[lineSize];
        for (int y = 0; y < _height; y++) {
            memset(lineBits, 0, sizeof(char) * lineSize);
            char* ptr = lineBits;
            for (int x = 0; x < _width; x++) {
                int idx = y * _width + x;
                RGBTriple triple;
                triple.rgbRed = toByte(_pixels[idx].red());
                triple.rgbGreen = toByte(_pixels[idx].green());
                triple.rgbBlue = toByte(_pixels[idx].blue());
                memcpy(ptr, &triple, sizeof(RGBTriple));
                ptr += sizeof(RGBTriple);
            }
            ofs.write(lineBits, sizeof(char) * lineSize);
        }
        delete[] lineBits;

        ofs.close();
    }

    double Image::toReal(unsigned char b) {
        static const double gamma = 2.2;
        double d = b / 255.0;
        return pow(d, gamma);
    }

    unsigned char Image::toByte(double d) {
        static const double invgamma = 1.0 / 2.2;
        d = std::max(0.0, std::min(d, 1.0));
        return (unsigned char)(255.0 * pow(d, invgamma));
    }
}
