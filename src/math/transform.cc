#define SPICA_API_EXPORT
#include "transform.h"

namespace spica {

    Transform::Transform()
        : m_{}
        , mInv_{} {
    }

    Transform::Transform(const double mat[4][4])
        : m_{ mat[0][0], mat[0][1], mat[0][2], mat[0][3],
              mat[1][0], mat[1][1], mat[1][2], mat[1][3],
              mat[2][0], mat[2][1], mat[2][2], mat[2][3],
              mat[3][0], mat[3][1], mat[3][2], mat[3][3] }
        , mInv_{ m_.inverted() } {
    }

    Transform::Transform(const Matrix4x4& m)
        : m_{m}
        , mInv_{m.inverted()} {
    }

    Transform::Transform(const Transform& t)
        : Transform{} {
        this->operator=(t);
    }

    Transform& Transform::operator=(const Transform& t) {
        if (this == &t) return *this;

        this->m_ = t.m_;
        this->mInv_ = t.mInv_;
        return *this;
    }

    bool Transform::operator==(const Transform& t) {
        return m_ == t.m_ && mInv_ == t.mInv_;
    }

    bool Transform::operator!=(const Transform& t) {
        return m_ != t.m_ || mInv_ != t.mInv_;
    }

    bool Transform::isIdentity() const {
        return (m_(0, 0) == 1.0 && m_(0, 1) == 0.0 && m_(0, 2) == 0.0 && m_(0, 3) == 0.0 &&
                m_(1, 0) == 0.0 && m_(1, 1) == 1.0 && m_(1, 2) == 0.0 && m_(1, 3) == 0.0 &&
                m_(2, 0) == 0.0 && m_(2, 1) == 0.0 && m_(2, 2) == 1.0 && m_(2, 3) == 0.0 &&
                m_(3, 0) == 0.0 && m_(3, 1) == 0.0 && m_(3, 2) == 0.0 && m_(3, 3) == 1.0);
    }

}  // namespace spica
