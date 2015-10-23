#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_IMAGE_H_
#define SPICA_IMAGE_H_

#include <string>
#include <memory>

#include "color.h"

namespace spica {

    enum class Tonemap {
        Rainhard = 0x00
    };

    /** Image class.
     */
    class SPICA_EXPORTS Image {
    private:
        unsigned int _width  = 0U;
        unsigned int _height = 0U;
        std::unique_ptr<Color[]> _pixels = {};

    public:
        Image();
        Image(unsigned int width, unsigned int height);
        Image(const Image& image);
        Image(Image&& image);

        virtual ~Image();

        Image& operator=(const Image& image);
        Image& operator=(Image&& image);

        static Image fromFile(const std::string& filename);

        const Color& operator()(int x, int y) const;
        Color& pixel(int x, int y);

        void resize(const int width, const int height);
        void fill(const Color& color);

        /** Gamma correction.
         *  @param[in] gamma: gamma value
         */
        void gammaCorrect(const double gamma);

        void load(const std::string& filename);
        void save(const std::string& filename) const;

        void tonemap(Tonemap algo = Tonemap::Rainhard);

        inline unsigned int width() const { return _width; }
        inline unsigned int height() const { return _height; }

    protected:
        /** Post save process.
         *  This can be mainly used in the overloaded class.
         */
        virtual void postSave() const {}

    private:
        void release();
        static double toReal(unsigned char b);
        static unsigned char toByte(double d);

        void loadBmp(const std::string& filename);
        void saveBmp(const std::string& filename) const;

        void loadHdr(const std::string& filename);
        void saveHdr(const std::string& filename) const;

        void loadPng(const std::string& filename);
        void savePng(const std::string& filename) const;
    };

}

#endif  // SPICA_IMAGE_H_
