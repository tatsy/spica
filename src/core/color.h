#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_COLOR_H_
#define SPICA_COLOR_H_

#include <iostream>

#include "common.h"

namespace spica {

    /** RGB color class.
     */
    class SPICA_EXPORTS Color {
    public:
        /** Black. */
        SPICA_IMPORTS static const Color BLACK;
        /** White. */
        SPICA_IMPORTS static const Color WHITE;
        /** Red. */
        SPICA_IMPORTS static const Color RED;
        /** Green. */
        SPICA_IMPORTS static const Color GREEN;
        /** Blue. */
        SPICA_IMPORTS static const Color BLUE;
        /** Cyan. */
        SPICA_IMPORTS static const Color CYAN;
        /** Yellow. */
        SPICA_IMPORTS static const Color YELLOW;
        /** Magenta. */
        SPICA_IMPORTS static const Color MAGENTA;

    private:
        double _r, _g, _b;

    public:
        /** The Color constructor. */
        Color();
        /** The Color constructor
         *  @param red: Red component.
         *  @param green: Green component.
         *  @param blue: Blue component.
         */
        Color(double red, double green, double blue);
        /** The Color constructor (copy) */
        Color(const Color& color);
        /** The Color destructor */
        ~Color();

        /** Assignment operator */
        Color& operator=(const Color& color);
        /** Plus operator */
        Color& operator+=(const Color& c);
        /** Plus operator.
         *  @details Add the same value x to each color component.
         */
        Color& operator+=(double x);
        /** Minus operator. */
        Color& operator-=(const Color& c);
        /** Minus operator.
         *  @details Subtract the same value x from each color component   
         */
        Color& operator-=(double x);
        /** Component-wise multiplication. */
        Color& operator*=(const Color& c);
        /** Scalar multiplication. */
        Color& operator*=(double s);
        /** Component-wise division */
        Color& operator/=(const Color& c);
        /** Scalar division */
        Color& operator/=(double s);
        /** Negation operator */
        Color operator-() const;

        static Color minimum(const Color& c1, const Color& c2);
        static Color maximum(const Color& c1, const Color& c2);

        static Color sqrt(const Color& c);
        static Color exp(const Color& c);
        static Color log(const Color& c);
        Color clamp(const Color& lo = Color(0.0, 0.0, 0.0),
                    const Color& hi = Color(INFTY, INFTY, INFTY)) const;

        double dot(const Color& color) const;
        double norm() const;
        double squaredNorm() const;
        double luminance() const;

        /** Red component. */
        inline double red()   const { return _r; }
        /** Green component */
        inline double green() const { return _g; }
        /** Blue component */
        inline double blue()  const { return _b; }

        /** Element accessor. */
        inline double operator[](int i) const {
            Assertion(i >= 0 && i <= 2, "Index out of range!!");
            if (i == 0) return _r;
            if (i == 1) return _g;
            if (i == 2) return _b;
            return 0.0;
        }

        /** Element accessor. */
        inline double& ref(int i) {
            Assertion(i >= 0 && i <= 2, "Index out of range!!");
            if (i == 0) return _r;
            if (i == 1) return _g;
            if (i == 2) return _b;            
        }

        /** Covert to string. */
        std::string toString() const;
    };

}  // namespace spica

SPICA_EXPORTS spica::Color operator+(const spica::Color& c1, const spica::Color& c2);
SPICA_EXPORTS spica::Color operator+(double x, const spica::Color& c);
SPICA_EXPORTS spica::Color operator+(const spica::Color& c, double x);
SPICA_EXPORTS spica::Color operator-(const spica::Color& c1, const spica::Color& c2);
SPICA_EXPORTS spica::Color operator-(double x, const spica::Color& c);
SPICA_EXPORTS spica::Color operator-(const spica::Color& c, double x);
SPICA_EXPORTS spica::Color operator*(const spica::Color& c1, const spica::Color& c2);
SPICA_EXPORTS spica::Color operator*(const spica::Color& c1, double s);
SPICA_EXPORTS spica::Color operator*(double s, const spica::Color& c2);
SPICA_EXPORTS spica::Color operator/(const spica::Color& c1, const spica::Color& c2);
SPICA_EXPORTS spica::Color operator/(const spica::Color& c1, double s);
SPICA_EXPORTS std::ostream& operator<<(std::ostream& os, const spica::Color& c);

#endif  // SPICA_COLOR_H_
