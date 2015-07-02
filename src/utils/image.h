#ifndef SPICA_IMAGE_H_
#define SPICA_IMAGE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_IMAGE_EXPORT
        #define SPICA_IMAGE_DLL __declspec(dllexport)
    #else
        #define SPICA_IMAGE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_IMAGE_DLL
#endif

#include <string>

#include "color.h"

namespace spica {

    enum TMAlgorithm {
        TM_REINHARD = 0x00
    };

    class SPICA_IMAGE_DLL Image {
    private:
        unsigned int _width;
        unsigned int _height;
        Color* _pixels;

    public:
        Image();
        Image(int width, int height);
        Image(const Image& image);
        Image(Image&& image);

        virtual ~Image();

        Image& operator=(const Image& image);
        Image& operator=(Image&& image);

        const Color& operator()(int x, int y) const;
        Color& pixel(int x, int y);

        void resize(const int width, const int height);
        void fill(const Color& color);

        // Gamma correction
        // @param[in] gam: gamma value
        // @param[in] inv: if true inverse gamma correction is performed
        void gamma(const double gam, bool inv = false);

        void loadBMP(const std::string& filename);
        virtual void saveBMP(const std::string& filename) const;

        void loadHDR(const std::string& filename);
        virtual void saveHDR(const std::string& filename) const;

        void tonemap(TMAlgorithm algo = TM_REINHARD);

        inline unsigned int width() const { return _width; }
        inline unsigned int height() const { return _height; }

    private:
        void release();
        static double toReal(unsigned char b);
        static unsigned char toByte(double d);
    };

}

#endif  // SPICA_IMAGE_H_
