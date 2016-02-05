#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MATRIX4X4_H_
#define _SPICA_MATRIX4X4_H_

#include "../core/common.h"

namespace spica {

    /**
     * Matrix of 4 x 4 size.
     * @ingroup math_module
     */
    class SPICA_EXPORTS Matrix4x4 {
    private:
        double m[4][4];

    public:
        /** 
         * Constructor.
         * @details
         * The constructor initializes the matrix as an identity.
         */
        Matrix4x4();

        Matrix4x4(double m[4][4]);
        Matrix4x4(double t00, double t01, double t02, double t03,
                  double t10, double t11, double t12, double t13,
                  double t20, double t21, double t22, double t23,
                  double t30, double t31, double t32, double t33);

        /**
         * Return the transposed matrix of the current instance.
         */
        Matrix4x4 transposed() const;

        /**
         * Return the inverted matrix of the current instance.
         * @details
         * This employs the simple Gauss elimination.
         */
        Matrix4x4 inverted() const;

        bool operator==(const Matrix4x4& m2) const;
        bool operator!=(const Matrix4x4& m2) const;
    };

}  // namespace spica

#endif  // _SPICA_MATRIX4X4_H_
