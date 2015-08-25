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

#include "vector3d.h"

namespace spica {

    class SPICA_COLOR_DLL Color : public Vector3D {

    public:
        static const Color BLACK;
        static const Color WHITE;
        static const Color RED;
        static const Color GREEN;
        static const Color BLUE;
        static const Color CYAN;
        static const Color YELLOW;
        static const Color MAGENTA;

    public:
        Color();
        Color(double red, double green, double blue);
        Color(const Color& color);
        Color(const Vector3D& v);

        ~Color();

        Color& operator=(const Color& color);
        Color& operator=(const Vector3D& v);

        double luminance() const;

        inline double red()   const { return this->x(); }
        inline double green() const { return this->y(); }
        inline double blue()  const { return this->z(); }
    };

}

#endif  // SPICA_COLOR_H_
