#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRANSFORM_H_
#define _SPICA_TRANSFORM_H_

#include "../core/common.h"
#include "../core/bounds3d.h"
#include "matrix4x4.h"

namespace spica {

/**
 * The transformation operator class.
 */
class SPICA_EXPORTS Transform {
public:
    // Public methods
    Transform();
    explicit Transform(const double mat[4][4]);
    explicit Transform(const Matrix4x4& m);
    Transform(const Matrix4x4& m, const Matrix4x4& mInv);
    Transform(const Transform& t);

    Transform& operator=(const Transform& t);

    bool operator==(const Transform& t);
    bool operator!=(const Transform& t);
    Transform& operator*=(const Transform& t);

    Point3d  apply(const Point3d&  p) const;
    Vector3d apply(const Vector3d& v) const;
    Normal3d apply(const Normal3d& n) const;
    Bounds3d  apply(const Bounds3d& b)  const;

    bool isIdentity() const;

    Transform inverted() const;
    static Transform mitsubaToPbrt(const Transform &t);

    inline const Matrix4x4& getMat() const { return m_; }
    inline const Matrix4x4& getInvMat() const { return mInv_; }

    // Public static methods
    static Transform translate(const Vector3d& delta);
    static Transform scale(double x, double y, double z);
    static Transform rotate(double theta, const Vector3d& axis);
    static Transform lookAt(const Point3d& eye, const Point3d& look,
                            const Vector3d& up);
    static Transform orthographic(double zNear, double zFar);
    static Transform perspective(double fov, double aspect, 
                                    double near, double far);

private:
    // Private fields
    Matrix4x4 m_, mInv_;
};

}  // namespace spica

SPICA_EXPORTS spica::Transform operator*(const spica::Transform& t1,
                           const spica::Transform& t2);

#endif  // _SPICA_TRANSFORM_H_
