#ifndef SPICA_COLOR_H_
#define SPICA_COLOR_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_COLOR_EXPORT
        #define SPICA_COLOR_DLL __declspec(dllexport)
    #else
        #define SPICA_COLOR_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_COLOR_DLL
#endif

#include "Vector3.h"

namespace spica {

    class SPICA_COLOR_DLL Color : public Vector3 {
    public:
        Color();
        Color(double red, double green, double blue);
        Color(const Color& color);
        Color(const Vector3& v);

        ~Color();

        Color& operator=(const Color& color);
        Color& operator=(const Vector3& v);

        inline double red() const { return _x; }
        inline double green() const { return _y; }
        inline double blue() const { return _z; }
    };

}

#endif  // SPICA_COLOR_H_
