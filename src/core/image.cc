#define SPICA_API_EXPORT
#include "image.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <algorithm>

#include "common.h"
#include "path.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../3rdparty/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../3rdparty/stb_image_write.h"

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
            unsigned char r = 0;
            unsigned char g = 0;
            unsigned char b = 0;
            unsigned char e = 0;
            
            HDRPixel() {
            }

            explicit HDRPixel(const Color& color)
                : HDRPixel{} {
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


    Image::Image() {
    }

    Image::Image(unsigned int width, unsigned int height)
        : _width{width}
        , _height{height} {
        Assertion(width >= 0 && height >= 0, "Image size must be positive");
        _pixels = std::make_unique<Color[]>(_width * _height);
    }

    Image::Image(const Image& image) 
        : Image{} {
        this->operator=(image);
    }

    Image::Image(Image&& image)
        : Image{} {
        this->operator=(std::move(image));
    }

    Image::~Image() {
    }

    void Image::release() {
        _width  = 0U;
        _height = 0U;
        _pixels.reset();
    }

    Image& Image::operator=(const Image& image) {
        if (this == &image) return *this;

        release();

        this->_width  = image._width;
        this->_height = image._height;
        this->_pixels = std::make_unique<Color[]>(_width * _height);
        std::copy(image._pixels.get(),
                  image._pixels.get() + (_width * _height),
                  _pixels.get());

        return *this;
    }

    Image& Image::operator=(Image&& image) {
        release();

        this->_width  = image._width;
        this->_height = image._height;
        this->_pixels = std::move(image._pixels);

        image._width  = 0;
        image._height = 0;

        return *this;
    }

    Image Image::fromFile(const std::string& filename) {
        Image img;
        img.load(filename);
        return std::move(img);
    }

    const Color& Image::operator()(int x, int y) const {
        Assertion(0 <= x && x < _width && 0 <= y && y < _height,
                  "Pixel index out of bounds");
        return _pixels[y * _width + x];
    }

    Color& Image::pixel(int x, int y) {
        Assertion(0 <= x && x < _width && 0 <= y && y < _height,
                  "Pixel index out of bounds");
        return _pixels[y * _width + x];
    }

    void Image::resize(const int width, const int height) {
        this->_width  = width;
        this->_height = height;

        _pixels.reset();
        _pixels = std::make_unique<Color[]>(width * height);
    }

    void Image::fill(const Color& color) {
        const int n = _width * _height;
        for (int i = 0; i < n; i++) {
            _pixels[i] = color;
        }
    }

    void Image::gammaCorrect(double gamma) {
        for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) {
                Color& c = _pixels[y * _width + x];
                double r = pow(c.red(),   gamma);
                double g = pow(c.green(), gamma);
                double b = pow(c.blue(),  gamma);
                c = Color(r, g, b);
            }
        }
    }

    void Image::load(const std::string& filename) {
        const std::string& ext = path::getExtension(filename);
        if (ext == ".bmp") {
            loadBmp(filename);
        } else if (ext == ".hdr") {
            loadHdr(filename);
        } else if (ext == ".png") {
            loadPng(filename);
        } else {
            fprintf(stderr, "[ERROR] unknown file extension: %s\n", ext.c_str());
            std::abort();
        }
    }

    void Image::save(const std::string& filename) const {
        const std::string& ext = path::getExtension(filename);
        if (ext == ".bmp") {
            saveBmp(filename);
        } else if (ext == ".hdr") {
            saveHdr(filename);
        } else if (ext == ".png") {
            savePng(filename);
        } else {
            fprintf(stderr, "[ERROR] unknown file extension: %s\n", ext.c_str());
            std::abort();
        }
        postSave();
    }

    void Image::loadBmp(const std::string& filename) {
        release();

        BitmapFileHeader header;
        BitmapCoreHeader core;

        std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "[ERROR] failed to load file \"" << filename << "\"" << std::endl;
            std::abort();
        }

        ifs.read((char*)&header, sizeof(BitmapFileHeader));
        ifs.read((char*)&core, sizeof(BitmapCoreHeader));

        this->_width  = std::abs(core.biWidth);
        this->_height = std::abs(core.biHeight);
        this->_pixels = std::make_unique<Color[]>(_width * _height);

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

    void Image::saveBmp(const std::string& filename) const {
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
        core.biHeight = -static_cast<int>(_height);
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

    void Image::loadHdr(const std::string& filename) {
        release();

        // Open file
        FILE* fp = fopen(filename.c_str(), "rb");
        if (fp == NULL) {
            std::cerr << "[ERROR] failed to load file \"" << filename << "\"" << std::endl;
            std::abort();
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
                    char temp[bufSize] = {0};
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
            fclose(fp);
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
            delete[] tmp_data;
            delete[] buffer;
            fclose(fp);
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
        this->_width  = width;
        this->_height = height;
        this->_pixels = std::make_unique<Color[]>(width * height);
        for (unsigned int y = 0; y < height; y++) {
            for (unsigned int x = 0; x < width; x++) {
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

    void Image::saveHdr(const std::string& filename) const {
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

        sprintf(buffer, "-Y %u +X %u%c", _height, _width, ret);
        ofs.write(buffer, strlen(buffer));

        std::vector<unsigned char> pixbuf;
        for (unsigned int i = 0; i < _height; i++) {
            std::vector<HDRPixel> line;
            for (unsigned int j = 0; j < _width; j++) {
                Color color = this->operator()(j, i);
                line.push_back(HDRPixel(color));
            }
            pixbuf.push_back(0x02);
            pixbuf.push_back(0x02);
            pixbuf.push_back((_width >> 8) & 0xff);
            pixbuf.push_back(_width & 0xff);
            for (int c = 0; c < 4; c++) {
                for (unsigned int cursor = 0; cursor < _width;) {
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

    void Image::loadPng(const std::string& filename) {
        int w, h, comp;
        unsigned char* data = stbi_load(filename.c_str(), &w, &h, &comp, STBI_rgb_alpha);
        
        this->_width  = w;
        this->_height = h;
        this->_pixels = std::make_unique<Color[]>(w * h);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                const double r = data[(y * w + x) * 4 + 0] / 255.0;             
                const double g = data[(y * w + x) * 4 + 1] / 255.0;             
                const double b = data[(y * w + x) * 4 + 2] / 255.0;             
                this->_pixels[y * w + x] = Color(r, g, b);
            }
        }
    }

    void Image::savePng(const std::string& filename) const {
        unsigned char* data = new unsigned char[_width * _height * 3];
        for (unsigned int y = 0; y < _height; y++) {
            for (unsigned int x = 0; x < _width; x++) {
                int idx = y * _width + x;
                data[idx * 3 + 0] = toByte(_pixels[idx].red());
                data[idx * 3 + 1] = toByte(_pixels[idx].green());
                data[idx * 3 + 2] = toByte(_pixels[idx].blue());
            }
        }
        stbi_write_png(filename.c_str(), _width, _height, 3, data, _width * 3);
        delete[] data;
    }

    void Image::tonemap(Tonemap algo) {
        Assertion(algo == Tonemap::Rainhard, "Tone mapping algorithm other than Reinhard '02 is not supported");

        const double delta = 1.0e-8;
        const double a = 0.18;

        double lw_bar = 0.0;
        double l_white = 0.0;
        for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) {
                Color c = this->operator()(x, y);
                double l = c.luminance();
                lw_bar += log(l + delta);         
                l_white = std::max(l_white, l);
            }
        }
        lw_bar = exp(lw_bar / (_width * _height));

        const double l_white2 = l_white * l_white;
        for (unsigned int y = 0; y < _height; y++) {
            for (unsigned int x = 0; x < _width; x++) {
                Color c = this->operator()(x, y);
                Color ret = c * (a / lw_bar);
                ret = ret * (1.0 + ret / l_white2) / (1.0 + ret);
                ret = Color::minimum(ret, Color(1.0, 1.0, 1.0));
                this->pixel(x, y) = ret;
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
