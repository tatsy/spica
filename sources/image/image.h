#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_IMAGE_H_
#define SPICA_IMAGE_H_

#include <string>
#include <memory>

#include "../core/spectrum.h"

namespace spica {

    /** 
     * Image class.
     */
    class SPICA_EXPORTS Image {
    public:
        Image();
        Image(unsigned int width, unsigned int height);
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

        inline unsigned int width() const { return _width; }
        inline unsigned int height() const { return _height; }

    protected:
        /** 
         * @brief Post save process.
         * @details 
         * This method is used, for example in the derived class,
         * when you want to notify another process that the image is saved.
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

    private:
        unsigned int _width  = 0U;
        unsigned int _height = 0U;
        std::unique_ptr<RGBSpectrum[]> _pixels = {};

    };

}

#endif  // SPICA_IMAGE_H_
