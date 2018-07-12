#define SPICA_API_EXPORT
#include "image.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "core/common.h"
#include "core/exception.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

    explicit HDRPixel(const RGBSpectrum& color)
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

}  // Anonymous namespace


Image::Image() {
}

Image::Image(int width, int height)
    : width_{width}
    , height_{height} {
    Assertion(width >= 0 && height >= 0, "Image size must be positive");
    pixels_ = std::make_unique<RGBSpectrum[]>(width_ * height_);
}

Image::Image(const Image& image)
    : Image{} {
    this->width_ = image.width_;
    this->height_ = image.height_;
    this->pixels_.reset(new RGBSpectrum[width_ * height_]);
    std::copy(image.pixels_.get(),
              image.pixels_.get() + (width_ * height_),
              pixels_.get());
}

Image::Image(Image&& image)
    : Image{} {
    image.swap(*this);
}

Image::~Image() {
}

Image &Image::operator=(const Image& image) {
    if (this == &image) return *this;

    Image temp(image);
    temp.swap(*this);
    return *this;
}

Image &Image::operator=(Image&& image) {
    if (this == &image) return *this;

    image.swap(*this);
    return *this;
}

Image Image::fromFile(const std::string& filename) {
    Image img;
    img.load(filename);
    return std::move(img);
}

const RGBSpectrum& Image::operator()(int x, int y) const {
    Assertion(0 <= x && x < width_ &&
              0 <= y && y < height_,
              "Pixel index out of bounds!");
    return pixels_[y * width_ + x];
}

RGBSpectrum& Image::pixel(int x, int y) {
    Assertion(0 <= x && x < width_ &&
              0 <= y && y < height_,
              "Pixel index out of bounds!");
    return pixels_[y * width_ + x];
}

void Image::resize(const int width, const int height) {
    this->width_ = width;
    this->height_ = height;
    pixels_.reset(new RGBSpectrum[width_ * height_]);
}

void Image::fill(const RGBSpectrum& color) {
    const int n = width_ * height_;
    std::fill(pixels_.get(), pixels_.get() + n, color);
}

void Image::swap(Image &image) {
    std::swap(width_, image.width_);
    std::swap(height_, image.height_);
    std::swap(pixels_, image.pixels_);
}

void Image::load(const std::string& filename) {
    fs::path path(filename);
    const std::string ext = path.extension().string();

    if (ext == "bmp") {
        loadBmp(filename);
    } else if (ext == "hdr") {
        loadHdr(filename);
    } else if (ext == "png") {
        loadPng(filename);
    } else {
        throw RuntimeException("Unknown file extension: %s", ext.c_str());
    }
}

void Image::save(const std::string& filename) const {
    fs::path path(filename);
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".bmp") {
        saveBmp(filename);
    } else if (ext == ".hdr") {
        saveHdr(filename);
    } else if (ext == ".png") {
        savePng(filename);
    } else {
        throw RuntimeException("Unknown file extension: %s", ext.c_str());
    }

    postSaveEvent();
}

void Image::loadBmp(const std::string& filename) {
    int w, h, comp;
    uint8_t *data = stbi_load(filename.c_str(), &w, &h, &comp, STBI_rgb_alpha);
    if (!data) {
        throw RuntimeException("Failed to open file: %s", filename.c_str());
    }

    this->width_  = w;
    this->height_ = h;
    this->pixels_ = std::make_unique<RGBSpectrum[]>(w * h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const double r = data[(y * w + x) * 4 + 0] / 255.0;
            const double g = data[(y * w + x) * 4 + 1] / 255.0;
            const double b = data[(y * w + x) * 4 + 2] / 255.0;
            pixels_[y * w + x] = RGBSpectrum(r, g, b);
        }
    }

    stbi_image_free(data);
}

void Image::saveBmp(const std::string& filename) const {
    uint8_t *data = new uint8_t[width_ * height_ * 3];
    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            const int i = y * width_ + x;
            data[i * 3 + 0] = toByte(pixels_[i].red());
            data[i * 3 + 1] = toByte(pixels_[i].green());
            data[i * 3 + 2] = toByte(pixels_[i].blue());
        }
    }
    stbi_write_bmp(filename.c_str(), width_, height_, 3, data);
    delete[] data;
}

void Image::loadHdr(const std::string& filename) {
    // Open file
    std::ifstream reader(filename.c_str(), std::ios::binary);
    if (!reader.is_open()) {
        throw RuntimeException("Failed to load file: %s", filename.c_str());
    }

    constexpr int bufSize = 4096;
    std::string line;

    HDRFileType fileType = HDR_NONE;
    bool isValid = false;
    double exposure = 1.0f;

    // Load header
    for (;;) {
        std::getline(reader, line);
        if (line[0] == '#') {
            if (line == "#?RADIANCE") {
                isValid = true;
            }
        } else {
            if (line.find("FORMAT=") != std::string::npos) {
                char temp[bufSize];
                if (std::sscanf(line.c_str(), "FORMAT=%128s", temp) != 1) {
                    throw RuntimeException("Invalid HDRI format: %s", line.c_str());
                }

                if (std::strcmp(temp, "32-bit_rle_rgbe") == 0) {
                    fileType = HDR_RLE_RGBE_32;
                }
            } else if (line.find("EXPOSURE=") != std::string::npos) {
                if (std::sscanf(line.c_str(), "EXPOSURE=%lf", &exposure) != 1) {
                    throw RuntimeException("Invalid exposure value: %s", line.c_str());
                }
            }
        }

        if (line.size() == 0) {
            break;
        }
    }

    // If the file is invalid, then return
    if (!isValid) {
        throw RuntimeException("Invalid HDR file: %s", filename.c_str());
    }

    // Load image size
    int width, height;
    char bufX[bufSize];
    char bufY[bufSize];
    std::getline(reader, line);
    if (std::sscanf(line.c_str(), "%16s %d %16s %d", bufY, &height, bufX, &width) != 4) {
        throw RuntimeException("Failed to parse HDRI size: %s", line.c_str());
    }

    if (std::strcmp(bufY, "-Y") != 0 || std::strcmp(bufX, "+X") != 0) {
        throw RuntimeException("Invalid HDR file: %s", filename.c_str());
    }
    auto tmp_data = std::make_unique<uint8_t[]>(width * height * 4);

    // Load image pixels
    const int64_t now_pos = reader.tellg();
    reader.seekg(0, reader.end);
    const int64_t end_pos = reader.tellg();
    reader.seekg(now_pos);

    const uint32_t rest_size = static_cast<uint32_t>(end_pos - now_pos);
    auto bytes = std::make_unique<uint8_t[]>(rest_size);
    reader.read((char*)&bytes[0], rest_size);

    int index = 0;
    int nowy = 0;
    for (; index < rest_size;) {
        const int now = bytes[index++];
        if (now == EOF) {
            break;
        }

        const int now2 = bytes[index++];
        if (now != 0x02 || now2 != 0x02) {
            break;
        }

        const int A = bytes[index++];
        const int B = bytes[index++];
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

            const int info = bytes[index++];
            if (info <= 128) {
                for (int i = 0; i < info; i++) {
                    const int data = bytes[index++];
                    const int pos = (nowy * width + nowx) * 4 + nowvalue;
                    tmp_data[pos] = static_cast<uint8_t>(data);
                    nowx++;
                }
            } else {
                const int num = info - 128;
                const int data = bytes[index++];
                for (int i = 0; i < num; i++) {
                    const int pos = (nowy * width + nowx) * 4 + nowvalue;
                    tmp_data[pos] = static_cast<uint8_t>(data);
                    nowx++;
                }
            }
        }
        nowy++;
    }

    // Copy loaded data to this object
    this->width_ = width;
    this->height_ = height;
    this->pixels_ = std::make_unique<RGBSpectrum[]>(width * height);
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            const int e = tmp_data[(y * width + x) * 4 + 3];
            const double r = tmp_data[(y * width + x) * 4 + 0] * pow(2.0, e - 128.0) / 256.0;
            const double g = tmp_data[(y * width + x) * 4 + 1] * pow(2.0, e - 128.0) / 256.0;
            const double b = tmp_data[(y * width + x) * 4 + 2] * pow(2.0, e - 128.0) / 256.0;
            pixels_[y * width + x] = spica::RGBSpectrum(r, g, b);
        }
    }

    reader.close();
}

void Image::saveHdr(const std::string& filename) const {
    std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::binary);
    if (!ofs.is_open()) {
        throw RuntimeException("Failed to open file: %s", filename.c_str());
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

    sprintf(buffer, "-Y %u +X %u%c", height_, width_, ret);
    ofs.write(buffer, strlen(buffer));

    std::vector<unsigned char> pixbuf;
    for (int i = 0; i < height_; i++) {
        std::vector<HDRPixel> line;
        for (int j = 0; j < width_; j++) {
            RGBSpectrum color = this->operator()(j, i);
            line.push_back(HDRPixel(color));
        }
        pixbuf.push_back(0x02);
        pixbuf.push_back(0x02);
        pixbuf.push_back((width_ >> 8) & 0xff);
        pixbuf.push_back(width_ & 0xff);
        for (int c = 0; c < 4; c++) {
            for (unsigned int cursor = 0; cursor < width_;) {
                const int cursor_move = std::min((unsigned int)127, width_ - cursor);
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
    uint8_t *data = stbi_load(filename.c_str(), &w, &h, &comp, STBI_rgb_alpha);
    if (!data) {
        throw RuntimeException("Failed to open file: %s", filename.c_str());
    }

    this->width_  = w;
    this->height_ = h;
    this->pixels_ = std::make_unique<RGBSpectrum[]>(w * h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const double r = data[(y * w + x) * 4 + 0] / 255.0;
            const double g = data[(y * w + x) * 4 + 1] / 255.0;
            const double b = data[(y * w + x) * 4 + 2] / 255.0;
            pixels_[y * w + x] = RGBSpectrum(r, g, b);
        }
    }

    stbi_image_free(data);
}

void Image::savePng(const std::string &filename) const {
    uint8_t *data = new uint8_t[width_ * height_ * 3];
    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            const int i = y * width_ + x;
            data[i * 3 + 0] = toByte(pixels_[i].red());
            data[i * 3 + 1] = toByte(pixels_[i].green());
            data[i * 3 + 2] = toByte(pixels_[i].blue());
        }
    }
    stbi_write_png(filename.c_str(), width_, height_, 3, data, width_ * 3);
    delete[] data;
}

double Image::toReal(uint8_t b) {
    return b / 255.0;
}

uint8_t Image::toByte(double d) {
    d = std::max(0.0, std::min(d, 1.0));
    return static_cast<uint8_t>(255.0 * d);
}

}  // namespace spica
