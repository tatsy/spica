#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRANSFORM_H_
#define _SPICA_TRANSFORM_H_

#include "../core/common.h"
#include "../core/bound3d.h"
#include "matrix4x4.h"

namespace spica {

    /**
     * The transformation operator class.
     */
    class SPICA_EXPORTS Transform {
    public:
        Transform();
        Transform(const double mat[4][4]);
        Transform(const Matrix4x4& m);
        Transform(const Matrix4x4& m, const Matrix4x4& mInv);
        Transform(const Transform& t);

        Transform& operator=(const Transform& t);

        bool operator==(const Transform& t);
        bool operator!=(const Transform& t);

        Point3D  apply(const Point3D& p)  const;
        Vector3D apply(const Vector3D& v) const;
        Bound3d  apply(const Bound3d& b)  const;

        bool isIdentity() const;

        Transform inverted() const;

        inline const Matrix4x4& getMat() const { return m_; }
        inline const Matrix4x4& getInvMat() const { return mInv_; }

    private:
        Matrix4x4 m_, mInv_;
    };

}  // namespace spica

#endif  // _SPICA_TRANSFORM_H_
