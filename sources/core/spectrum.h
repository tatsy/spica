#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SPECTRUM_H_
#define _SPICA_SPECTRUM_H_

#include <iostream>
#include <vector>

#include "common.h"

namespace spica {

    /** 
     * RGB spectrum.
     */
    class SPICA_EXPORTS RGBSpectrum {
    public:
        /** The RGBSpectrum constructor. */
        RGBSpectrum();
        explicit RGBSpectrum(double l);
        
        /** The RGBSpectrum constructor.
         *  @param red: Red component.
         *  @param green: Green component.
         *  @param blue: Blue component.
         */
        RGBSpectrum(double red, double green, double blue);

        /** The RGBSpectrum constructor.
         */
        RGBSpectrum(const std::vector<double>& nm,
                    const std::vector<double>& values);

        /** The RGBSpectrum constructor (copy) */
        RGBSpectrum(const RGBSpectrum& RGBSpectrum);
        /** The RGBSpectrum destructor */
        ~RGBSpectrum();

        /** Assignment operator */
        RGBSpectrum& operator=(const RGBSpectrum& RGBSpectrum);
        /** Plus operator */
        RGBSpectrum& operator+=(const RGBSpectrum& c);
        /** Plus operator.
         *  @details Add the same value x to each RGBSpectrum component.
         */
        RGBSpectrum& operator+=(double x);
        /** Minus operator. */
        RGBSpectrum& operator-=(const RGBSpectrum& c);
        /** Minus operator.
         *  @details Subtract the same value x from each RGBSpectrum component   
         */
        RGBSpectrum& operator-=(double x);
        /** Component-wise multiplication. */
        RGBSpectrum& operator*=(const RGBSpectrum& c);
        /** Scalar multiplication. */
        RGBSpectrum& operator*=(double s);
        /** Component-wise division */
        RGBSpectrum& operator/=(const RGBSpectrum& c);
        /** Scalar division */
        RGBSpectrum& operator/=(double s);
        /** Negation operator */
        RGBSpectrum operator-() const;

        static RGBSpectrum fromXYZ(double x, double y, double z);

        static RGBSpectrum minimum(const RGBSpectrum& c1, const RGBSpectrum& c2);
        static RGBSpectrum maximum(const RGBSpectrum& c1, const RGBSpectrum& c2);

        static RGBSpectrum sqrt(const RGBSpectrum& c);
        static RGBSpectrum exp(const RGBSpectrum& c);
        static RGBSpectrum log(const RGBSpectrum& c);
        static RGBSpectrum clamp(const RGBSpectrum& c,
                                 const RGBSpectrum& lo = RGBSpectrum(0.0, 0.0, 0.0),
                                 const RGBSpectrum& hi = RGBSpectrum(INFTY, INFTY, INFTY));

        bool  isBlack() const;
        double dot(const RGBSpectrum& RGBSpectrum) const;
        double norm() const;
        double squaredNorm() const;
        double luminance() const;

        /** Red component. */
        inline double red()   const { return r_; }
        /** Green component */
        inline double green() const { return g_; }
        /** Blue component */
        inline double blue()  const { return b_; }

        /** Element accessor. */
        inline double operator[](int i) const {
            Assertion(i >= 0 && i <= 2, "Index out of range!!");
            if (i == 0) return r_;
            if (i == 1) return g_;
            if (i == 2) return b_;
            return 0.0;
        }

        /** Element accessor. */
        inline double& ref(int i) {
            Assertion(i >= 0 && i <= 2, "Index out of range!!");
            if (i == 0) return r_;
            if (i == 1) return g_;
            return b_;            
        }

        /** Convert to RGB spcetrum. */
        inline RGBSpectrum toRGB() const {
            return { r_, g_, b_ };
        }

        /** Covert to string. */
        std::string toString() const;

        // Public fields
        static const int channels = 3;

    private:
        double r_, g_, b_;
    };

    using Spectrum = RGBSpectrum;

}  // namespace spica

SPICA_EXPORTS spica::RGBSpectrum operator+(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2);
SPICA_EXPORTS spica::RGBSpectrum operator+(double x, const spica::RGBSpectrum& c);
SPICA_EXPORTS spica::RGBSpectrum operator+(const spica::RGBSpectrum& c, double x);
SPICA_EXPORTS spica::RGBSpectrum operator-(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2);
SPICA_EXPORTS spica::RGBSpectrum operator-(double x, const spica::RGBSpectrum& c);
SPICA_EXPORTS spica::RGBSpectrum operator-(const spica::RGBSpectrum& c, double x);
SPICA_EXPORTS spica::RGBSpectrum operator*(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2);
SPICA_EXPORTS spica::RGBSpectrum operator*(const spica::RGBSpectrum& c1, double s);
SPICA_EXPORTS spica::RGBSpectrum operator*(double s, const spica::RGBSpectrum& c2);
SPICA_EXPORTS spica::RGBSpectrum operator/(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2);
SPICA_EXPORTS spica::RGBSpectrum operator/(const spica::RGBSpectrum& c1, double s);
SPICA_EXPORTS std::ostream& operator<<(std::ostream& os, const spica::RGBSpectrum& c);

#endif  // SPICA_RGBSpectrum_H_
