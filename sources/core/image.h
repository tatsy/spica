#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_IMAGE_H
#define SPICA_IMAGE_H

#include <string>
#include <memory>

#include "spectrum.h"

namespace spica {

/**
 * Image class.
 */
class SPICA_EXPORTS Image {
public:
    Image();
    Image(int width, int height);
    Image(const Image& image);
    Image(Image&& image);

    virtual ~Image();

    Image& operator=(const Image& image);
    Image& operator=(Image&& image);

    static Image fromFile(const std::string& filename);

    const RGBSpectrum& operator()(int x, int y) const;
    RGBSpectrum& pixel(int x, int y);

    void resize(const int width, const int height);
    void fill(const RGBSpectrum& color);

    void load(const std::string& filename);
    void save(const std::string& filename) const;

    inline int width() const { return width_; }
    inline int height() const { return height_; }

protected:
    /**
     * @brief Post save event.
     * @details
     * This method is used, for example in the derived class,
     * when you want to notify another process that the image is saved.
     */
    virtual void postSaveEvent() const {}

private:
    void swap(Image &image);
    static double toReal(uint8_t b);
    static uint8_t toByte(double d);

    void loadBmp(const std::string& filename);
    void saveBmp(const std::string& filename) const;

    void loadHdr(const std::string& filename);
    void saveHdr(const std::string& filename) const;

    void loadPng(const std::string& filename);
    void savePng(const std::string& filename) const;

private:
    int width_  = 0;
    int height_ = 0;
    std::unique_ptr<RGBSpectrum[]> pixels_ = nullptr;
};

}  // namespace spica

#endif  // SPICA_IMAGE_H
