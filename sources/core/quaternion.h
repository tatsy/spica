#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_QUATERNION_H_
#define _SPICA_QUATERNION_H_

#include <iostream>

#include "core/core.hpp"

#include "core/common.h"

namespace spica {

    /** Quaternion.
     *  @ingroup math_module
     */
    class SPICA_EXPORTS Quaternion {
    private:
        double _x, _y, _z, _w;

    public:
        /** The Quaternion constructor.
         */
        Quaternion();

        /** The Quaternion constructor.
         *  @param x: The first imaginary part.
         *  @param y: The second imaginary part.
         *  @param z: The third imaginary part.
         *  @param w: The real part.
         */
        Quaternion(double x, double y, double z, double w = 0.0);

        /** The Quaternion constructor.
         *  @param v: The three-dimensional vector for imaginary parts.
         */
        explicit Quaternion(const Vector3d& v);

        /** The Quaternion constructor (copy).
         */
        Quaternion(const Quaternion& q);

        /** The Quaternion destructor.
         */
        ~Quaternion();

        /** Assignment operator.
         */
        Quaternion& operator=(const Quaternion& q);

        /** Multiplication operator.
         */
        Quaternion& operator*=(const Quaternion& q);

        /** Make instance from rotation angle.
         *  @param axis: Rotation axis
         *  @param theta: Rotation angle by radii
         */
        static Quaternion rotation(const Vector3d& axis, double theta);

        /** Apply transformation of quaternion to the vector.
         */
        Vector3d applyTo(const Vector3d& v);
        Point3d  applyTo(const Point3d& p);
        Normal3d applyTo(const Normal3d& n);

        /** Squared norm.
         */
        double squaredNorm() const;

        /** Norm.
         */
        double norm() const;

        /** Inverse quarternion.
         */
        Quaternion inverse() const;

        /** Extract imaginary parts as a three-dimensional vector.
         */
        Vector3d toVector3d() const;

        /** Convert to string
         */
        std::string toString() const;

        /** The first imaginary part. */
        inline double x() const { return _x; }

        /** The second imaginary part. */
        inline double y() const { return _y; }

        /** The third imaginary part. */
        inline double z() const { return _z; }

        /** The real part. */
        inline double w() const { return _w; }
    };

}  // namespace spica

SPICA_EXPORTS
spica::Quaternion operator*(const spica::Quaternion& q1,
                            const spica::Quaternion& q2);

SPICA_EXPORTS
std::ostream& operator<<(std::ostream& os, const spica::Quaternion& q);

#endif  // _SPICA_QUATERNION_H_
