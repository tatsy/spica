#ifdef _MSC_VER
#pragma once
#endif

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

#include <iostream>

#include "common.h"

namespace spica {

    class SPICA_COLOR_DLL Color {
    public:
        static const Color BLACK;
        static const Color WHITE;
        static const Color RED;
        static const Color GREEN;
        static const Color BLUE;
        static const Color CYAN;
        static const Color YELLOW;
        static const Color MAGENTA;

    private:
        double _r, _g, _b;

    public:
        Color();
        Color(double red, double green, double blue);
        Color(const Color& color);
        ~Color();


        Color& operator=(const Color& color);
        Color& operator+=(const Color& c);
        Color& operator+=(double x);
        Color& operator-=(const Color& c);
        Color& operator-=(double x);
        Color& operator*=(const Color& c);
        Color& operator*=(double s);
        Color& operator/=(const Color& c);
        Color& operator/=(double s);
        Color operator-() const;

        static Color minimum(const Color& c1, const Color& c2);
        static Color maximum(const Color& c1, const Color& c2);

        static Color sqrt(const Color& c);
        static Color exp(const Color& c);
        Color clamp(const Color& lo = Color(0.0, 0.0, 0.0),
                    const Color& hi = Color(INFTY, INFTY, INFTY)) const;

        double dot(const Color& color) const;
        double norm() const;
        double squaredNorm() const;
        double luminance() const;

        inline double red()   const { return _r; }
        inline double green() const { return _g; }
        inline double blue()  const { return _b; }
    
        std::string toString() const;
    };

}  // namespace spica

SPICA_COLOR_DLL spica::Color operator+(const spica::Color& c1, const spica::Color& c2);
SPICA_COLOR_DLL spica::Color operator+(double x, const spica::Color& c);
SPICA_COLOR_DLL spica::Color operator+(const spica::Color& c, double x);
SPICA_COLOR_DLL spica::Color operator-(const spica::Color& c1, const spica::Color& c2);
SPICA_COLOR_DLL spica::Color operator-(double x, const spica::Color& c);
SPICA_COLOR_DLL spica::Color operator-(const spica::Color& c, double x);
SPICA_COLOR_DLL spica::Color operator*(const spica::Color& c1, const spica::Color& c2);
SPICA_COLOR_DLL spica::Color operator*(const spica::Color& c1, double s);
SPICA_COLOR_DLL spica::Color operator*(double s, const spica::Color& c2);
SPICA_COLOR_DLL spica::Color operator/(const spica::Color& c1, const spica::Color& c2);
SPICA_COLOR_DLL spica::Color operator/(const spica::Color& c1, double s);
SPICA_COLOR_DLL std::ostream& operator<<(std::ostream& os, const spica::Color& c);

#endif  // SPICA_COLOR_H_
