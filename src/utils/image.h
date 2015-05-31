#ifndef SPICA_IMAGE_H_
#define SPICA_IMAGE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_IMAGE_EXPORT
        #define SPICA_IMAGE_DLL __declspec(dllexport)
    #else
        #define SPICA_IMAGE_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_IMAGE_DLL
#endif

#include <string>

#include "color.h"

namespace spica {

    class SPICA_IMAGE_DLL Image {
    private:
        unsigned int _width;
        unsigned int _height;
        Color* _pixels;

    public:
        Image();
        Image(int width, int height);
        Image(const Image& image);

        virtual ~Image();

        Image& operator=(const Image& image);

        const Color& operator()(int x, int y) const;
        Color& pixel(int x, int y);

        void loadBMP(const std::string& filename);
        void saveBMP(const std::string& filename) const;

        void loadHDR(const std::string& filename);

        inline unsigned int width() const { return _width; }
        inline unsigned int height() const { return _height; }

    private:
        void release();
        static double toReal(unsigned char b);
        static unsigned char toByte(double d);
    };

}

#endif  // SPICA_IMAGE_H_
