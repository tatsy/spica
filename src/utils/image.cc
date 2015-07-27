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
#include <vector>
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
            int biWidth;
            int biHeight;
            unsigned short biPlanes;
            unsigned short biBitCount;
            unsigned int   biCompression;
            unsigned int   biSizeImage;
            int biXPixPerMeter;
            int biYPixPerMeter;
            unsigned int  biClrUsed;
            unsigned int  biClrImportant;
        });

        PACKED(
        struct RGBTriple {
            unsigned char rgbBlue;
            unsigned char rgbGreen;
            unsigned char rgbRed;
        });

        enum HDRFileType {
            HDR_NONE,
            HDR_RLE_RGBE_32
        };

        struct HDRPixel {
            unsigned char r, g, b, e;
            
            HDRPixel()
                : r(0)
                , g(0)
                , b(0)
                , e(0)
            {
            }

            explicit HDRPixel(const Color& color)
                : r(0)
                , g(0)
                , b(0)
                , e(0)
            {
                double d = std::max(color.red(), std::max(color.green(), color.blue()));
                if (d <= 1.0e-32) {
                    r = g = b = e = 0;
                    return;
                }

                int ie;
                const double m = frexp(d, &ie);
                d = m * 256.0 / d;

                r = static_cast<unsigned char>(color.red() * d);
                g = static_cast<unsigned char>(color.green() * d);
                b = static_cast<unsigned char>(color.blue() * d);
                e = static_cast<unsigned char>(ie + 128);
            }

            unsigned char get(int idx) const {
                switch (idx) {
                case 0: return r;
                case 1: return g;
                case 2: return b;
                case 3: return e;
                }
                return 0;
            }
        };
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
        : _width(0)
        , _height(0)
        , _pixels(NULL)
    {
        this->operator=(image);
    }

    Image::Image(Image&& image)
        : _width(0)
        , _height(0)
        , _pixels(NULL)
    {
        this->operator=(std::move(image));
    }

    Image::~Image()
    {
        delete[] _pixels;
    }

    void Image::release() {
        this->_width = 0;
        this->_height = 0;
        delete[] _pixels;
        _pixels = NULL;
    }

    Image& Image::operator=(const Image& image) {
        release();

        this->_width = image._width;
        this->_height = image._height;
        this->_pixels = new Color[image._width * image._height];
        memcpy((void*)_pixels, (void*)image._pixels, sizeof(Color) * image._width * image._height);

        return *this;
    }

    Image& Image::operator=(Image&& image) {
        release();

        this->_width  = image._width;
        this->_height = image._height;
        this->_pixels = image._pixels;

        image._width = 0;
        image._height = 0;
        image._pixels = nullptr;

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

    void Image::resize(const int width, const int height) {
        this->_width = width;
        this->_height = height;

        delete[] _pixels;
        _pixels = new Color[width * height];
    }

    void Image::fill(const Color& color) {
        const int n = _width * _height;
        for (int i = 0; i < n; i++) {
            _pixels[i] = color;
        }
    }

    void Image::gamma(const double gam, bool inv) {
        const double gg = inv ? 1.0 / gam : gam;
        for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) {
                Color& c = _pixels[y * _width + x];
                double r = pow(c.red(),   gg);
                double g = pow(c.green(), gg);
                double b = pow(c.blue(),  gg);
                c = Color(r, g, b);
            }
        }
    }

    void Image::loadBMP(const std::string& filename) {
        release();

        BitmapFileHeader header;
        BitmapCoreHeader core;

        std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
        msg_assert(ifs.is_open(), "Failed to open file!!");

        ifs.read((char*)&header, sizeof(BitmapFileHeader));
        ifs.read((char*)&core, sizeof(BitmapCoreHeader));

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
                memcpy((void*)&triple, (void*)ptr, sizeof(RGBTriple));

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
                triple.rgbRed   = toByte(_pixels[idx].red());
                triple.rgbGreen = toByte(_pixels[idx].green());
                triple.rgbBlue  = toByte(_pixels[idx].blue());
                memcpy((void*)ptr, (void*)&triple, sizeof(RGBTriple));
                ptr += sizeof(RGBTriple);
            }
            ofs.write(lineBits, sizeof(char) * lineSize);
        }
        delete[] lineBits;

        ofs.close();
    }

    void Image::loadHDR(const std::string& filename) {
        release();

        // Open file
        FILE* fp = fopen(filename.c_str(), "rb");
        if (fp == NULL) {
            std::cerr << "Failed to load file \"" << filename << "\"" << std::endl;
            return;
        }

        const int bufSize = 4096;
        char buf[bufSize];

        HDRFileType fileType = HDR_NONE;
        bool isValid = false;
        float exposure = 1.0;

        // Load header
        for (;;) {
            fgets(buf, bufSize, fp);
            if (buf[0] == '#') {
                if (strcmp(buf, "#?RADIANCE\n") == 0) {
                    isValid = true;
                } 
            } else {
                if (strstr(buf, "FORMAT=") == buf) {
                    char temp[bufSize];
                    sscanf(buf, "FORMAT=%s", temp);
                    if (strcmp(temp, "32-bit_rle_rgbe") == 0) {
                        fileType = HDR_RLE_RGBE_32;
                    }
                } else if (strstr(buf, "EXPOSURE=") == buf) {
                    sscanf(buf, "EXPOSURE=%f", &exposure);
                }
            }

            if (buf[0] == '\n') {
                break;
            }
        }

        // If the file is invalid, then return
        if (!isValid) {
            std::cerr << "Invalid HDR file format: " << filename << std::endl;
            return;
        }

        // Load image size
        int width, height;
        char bufX[bufSize];
        char bufY[bufSize];
        fgets(buf, bufSize, fp);
        sscanf(buf, "%s %d %s %d", bufY, &height, bufX, &width);
        
        if (strcmp(bufY, "-Y") != 0 || strcmp(bufX, "+X") != 0) {
            std::cerr << "Invalid HDR file format: " << filename << std::endl;
        }
        unsigned char* tmp_data = new unsigned char[width * height * 4];

        // Load image pixels
        long now_pos = ftell(fp);
        fseek(fp, 0, SEEK_END);
        long end_pos = ftell(fp);
        fseek(fp, now_pos, SEEK_SET);

        const long rest_size = end_pos - now_pos;
        unsigned char* buffer = new unsigned char[rest_size];

        const size_t ret_size = fread(buffer, sizeof(unsigned char), rest_size, fp);
        if (ret_size < rest_size) {
            std::cerr << "Error: fread" << std::endl;
            return;
        }

        int index = 0;
        int nowy = 0;
        for (; index < rest_size;) {
            const int now = buffer[index++];
            if (now == EOF) {
                break;
            }

            const int now2 = buffer[index++];
            if (now != 0x02 || now2 != 0x02) {
                break;
            }

            const int A = buffer[index++];
            const int B = buffer[index++];
            const int width = (A << 8) | B;

            int nowx = 0;
            int nowvalue = 0;
            for (;;) {
                if (nowx >= width) {
                    nowvalue++;
                    nowx = 0;
                    if (nowvalue == 4) {
                        break;
                    }
                }

                const int info = buffer[index++];
                if (info <= 128) {
                    for (int i = 0; i < info; i++) {
                        const int data = buffer[index++];
                        tmp_data[(nowy * width + nowx) * 4 + nowvalue] = data;
                        nowx++;
                    }
                } else {
                    const int num = info - 128;
                    const int data = buffer[index++];
                    for (int i = 0; i < num; i++) {
                        tmp_data[(nowy * width + nowx) * 4 + nowvalue] = data;
                        nowx++;
                    }
                }
            }
            nowy++;
        }

        // Copy loaded data to this object
        this->_width = width;
        this->_height = height;
        this->_pixels = new spica::Color[width * height];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const int e = tmp_data[(y * width + x) * 4 + 3];
                const double r = tmp_data[(y * width + x) * 4 + 0] * pow(2.0, e - 128.0) / 256.0;
                const double g = tmp_data[(y * width + x) * 4 + 1] * pow(2.0, e - 128.0) / 256.0;
                const double b = tmp_data[(y * width + x) * 4 + 2] * pow(2.0, e - 128.0) / 256.0;
                _pixels[y * width + x] = spica::Color(r, g, b);
            }
        }

        fclose(fp);
        delete[] tmp_data;
        delete[] buffer;
    }

    void Image::saveHDR(const std::string& filename) const {
        std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open file \"" << filename << "\"" << std::endl;
            return;
        }

        char buffer[256];
        unsigned char ret = 0x0a;
        sprintf(buffer, "#?RADIANCE%c", ret);
        ofs.write(buffer, strlen(buffer));
        sprintf(buffer, "# Made with 100%% pure HDR Shop%c", ret);
        ofs.write(buffer, strlen(buffer));
        sprintf(buffer, "FORMAT=32-bit_rle_rgbe%c", ret);
        ofs.write(buffer, strlen(buffer));
        sprintf(buffer, "EXPOSURE=1.0000000000000%c%c", ret, ret);
        ofs.write(buffer, strlen(buffer));

        sprintf(buffer, "-Y %d +X %d%c", _height, _width, ret);
        ofs.write(buffer, strlen(buffer));

        std::vector<unsigned char> pixbuf;
        for (int i = 0; i < _height; i++) {
            std::vector<HDRPixel> line;
            for (int j = 0; j < _width; j++) {
                Color color = this->operator()(j, i);
                line.push_back(HDRPixel(color));
            }
            pixbuf.push_back(0x02);
            pixbuf.push_back(0x02);
            pixbuf.push_back((_width >> 8) & 0xff);
            pixbuf.push_back(_width & 0xff);
            for (int c = 0; c < 4; c++) {
                for (int cursor = 0; cursor < _width;) {
                    const int cursor_move = std::min((unsigned int)127, _width - cursor);
                    pixbuf.push_back(cursor_move);
                    for (int j = cursor; j < cursor + cursor_move; j++) {
                        pixbuf.push_back(line[j].get(c));
                    }
                    cursor += cursor_move;
                }
            }
        }
        ofs.write((char*)&pixbuf[0], pixbuf.size());

        ofs.close();
    }

    void Image::tonemap(TMAlgorithm algo) {
        msg_assert(algo == TM_REINHARD, "Tone mapping algorithm other than Reinhard '02 is not supported");

        const double delta = 1.0e-8;
        const double a = 0.18;

        double Lw_bar = 0.0;
        double L_white = 0.0;
        for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) {
                Color c = this->operator()(x, y);
                double l = c.luminance();
                Lw_bar += log(l + delta);         
                L_white = std::max(L_white, l);
            }
        }
        Lw_bar = exp(Lw_bar / (_width * _height));

        const double L_white2 = L_white * L_white;
        for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) {
                Color c = this->operator()(x, y);
                double r = c.red() * a / Lw_bar;
                r = r * (1.0 + r / L_white2) / (1.0 + r);
                double g = c.green() * a / Lw_bar;
                g = g * (1.0 + g / L_white2) / (1.0 + g);
                double b = c.blue() * a / Lw_bar;
                b = b * (1.0 + b / L_white2) / (1.0 + b);
                this->pixel(x, y) = Color(r, g, b);
            }
        }
    }

    double Image::toReal(unsigned char b) {
        return b / 255.0;
    }

    unsigned char Image::toByte(double d) {
        d = std::max(0.0, std::min(d, 1.0));
        return (unsigned char)(255.0 *d);
    }
}
