#define SPICA_API_EXPORT
#include "transform.h"

#include "vector3d.h"

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
        : m_{ m }
        , mInv_{ m.inverted() } {
    }

    Transform::Transform(const Matrix4x4& m, const Matrix4x4& mInv)
        : m_{ m }
        , mInv_{ mInv } {
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

    Transform& Transform::operator*=(const Transform& t) {
        m_ = m_ * t.m_;
        mInv_ = t.mInv_ * mInv_;
        return *this;
    }

    Point3D Transform::apply(const Point3D& p) const {
        double ps[4] = { p[0], p[1], p[2], 1.0 };
        
        double ret[4] = { 0.0, 0.0, 0.0, 0.0 };
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                ret[i] += m_(i, j) * ps[j]; 
            }
        }

        if (ret[3] != 1.0) {
            ret[0] /= (ret[3] + EPS);
            ret[1] /= (ret[3] + EPS);
            ret[2] /= (ret[3] + EPS);
        }
        return { ret[0], ret[1], ret[2] };
    }

    Vector3D Transform::apply(const Vector3D& v) const {
        return Vector3D(
            m_(0, 0) * v.x() + m_(0, 1) * v.y() + m_(0, 2) * v.z(),
            m_(1, 0) * v.x() + m_(1, 1) * v.y() + m_(1, 2) * v.z(),
            m_(2, 0) * v.x() + m_(2, 1) * v.y() + m_(2, 2) * v.z());
    }

    Bounds3d Transform::apply(const Bounds3d& b) const {
        Point3D posMin = apply(b.posMin());
        Point3D posMax = apply(b.posMax());
        return { posMin, posMax };
    }

    bool Transform::isIdentity() const {
        return (m_(0, 0) == 1.0 && m_(0, 1) == 0.0 && m_(0, 2) == 0.0 && m_(0, 3) == 0.0 &&
                m_(1, 0) == 0.0 && m_(1, 1) == 1.0 && m_(1, 2) == 0.0 && m_(1, 3) == 0.0 &&
                m_(2, 0) == 0.0 && m_(2, 1) == 0.0 && m_(2, 2) == 1.0 && m_(2, 3) == 0.0 &&
                m_(3, 0) == 0.0 && m_(3, 1) == 0.0 && m_(3, 2) == 0.0 && m_(3, 3) == 1.0);
    }

    Transform Transform::inverted() const {
        return Transform(mInv_, m_);
    }

    Transform Transform::translate(const Vector3D& delta) {
        Matrix4x4 m(1.0, 0.0, 0.0, delta.x(),
                    0.0, 1.0, 0.0, delta.y(),
                    0.0, 0.0, 1.0, delta.z(),
                    0.0, 0.0, 0.0, 1.0);
        Matrix4x4 mInv(1.0, 0.0, 0.0, -delta.x(),
                       0.0, 1.0, 0.0, -delta.y(),
                       0.0, 0.0, 1.0, -delta.z(),
                       0.0, 0.0, 0.0, 1.0);
        return Transform{ m, mInv };
    }

    Transform Transform::scale(double sx, double sy, double sz) {
        Assertion(sx != 0.0 && sy != 0.0 && sz != 0.0,
                  "Zero division!!");
        Matrix4x4 m( sx, 0.0, 0.0, 0.0,
                    0.0,  sy, 0.0, 0.0,
                    0.0, 0.0,  sz, 0.0,
                    0.0, 0.0, 0.0, 1.0);
        Matrix4x4 mInv(1.0 / sx, 0.0, 0.0, 0.0,
                       0.0, 1.0 / sy, 0.0, 0.0,
                       0.0, 0.0, 1.0 / sz, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        return Transform{ m, mInv };
    }

    Transform Transform::rotate(double theta, const Vector3D& axis) {
        Vector3D a = axis.normalized();
        double sinTheta = sin(theta);
        double cosTheta = cos(theta);
        double m[4][4];
        memset(m, 0, sizeof(m));

        m[0][0] = a.x() * a.x() + (1.0 - a.x() * a.x()) * cosTheta;
        m[0][1] = a.x() * a.y() * (1.0 - cosTheta) - a.z() * sinTheta;
        m[0][2] = a.x() * a.z() * (1.0 - cosTheta) + a.y() * sinTheta;
        m[0][3] = 0.0;

        m[1][0] = a.x() * a.y() * (1.0 - cosTheta) + a.z() * sinTheta;
        m[1][1] = a.y() * a.y() + (1.0 - a.y() * a.y()) * cosTheta;
        m[1][2] = a.x() * a.z() * (1.0 - cosTheta) + a.y() * sinTheta;
        m[1][3] = 0.0;

        m[2][0] = a.x() * a.z() * (1.0 - cosTheta) - a.y() * sinTheta;
        m[2][1] = a.y() * a.z() * (1.0 - cosTheta) + a.x() * sinTheta;
        m[2][2] = a.z() * a.z() + (1.0 - a.z() * a.z()) * cosTheta;
        m[2][3] = 0.0;
        
        Matrix4x4 mat(m);
        return Transform{ mat, mat.transposed() };
    }

    Transform Transform::lookAt(const Point3D& eye, const Point3D& look,
                                const Vector3D& up) {
        double c2w[4][4];
        memset(c2w, 0, sizeof(c2w));
        c2w[0][3] = eye.x();
        c2w[1][3] = eye.y();
        c2w[2][3] = eye.z();
        c2w[3][3] = 1.0;

        Vector3D dir = (look - eye).normalized();
        Vector3D left = Vector3D::cross(up.normalized(), dir);
        Assertion(left.norm() != 0.0,
                  "Up vector and viewing direction are oriented "
                  "the same direction!!");
        
        left = left.normalized();
        Vector3D newUp = Vector3D::cross(dir, left);
        c2w[0][0] = left.x();
        c2w[1][0] = left.y();
        c2w[2][0] = left.z();
        c2w[3][0] = 0.0;
        c2w[0][1] = newUp.x();
        c2w[1][1] = newUp.y();
        c2w[2][1] = newUp.z();
        c2w[3][1] = 0.0;
        c2w[0][2] = dir.x();
        c2w[1][2] = dir.y();
        c2w[2][2] = dir.z();
        c2w[3][2] = 0.0;

        Matrix4x4 cameraToWorld(c2w);
        return Transform{ cameraToWorld.inverted(), cameraToWorld };
    }

    Transform Transform::orthographic(double zNear, double zFar) {
        return scale(1.0, 1.0, 1.0 / (zFar - zNear)) * 
               translate(Vector3D(0.0, 0.0, -zNear));
    }

    Transform Transform::perspective(double fov, double aspect, double near, double far) {
        Matrix4x4 pers(1.0 / aspect, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, far / (far - near), -far * near / (far - near),
                       0.0, 0.0, 1.0, 0.0);
        double invTanHalf = 1.0 / tan(fov / 2.0);
        return scale(invTanHalf, invTanHalf, 1.0) * Transform(pers);
    }

}  // namespace spica

spica::Transform operator*(const spica::Transform& t1,
                           const spica::Transform& t2) {
    spica::Transform ret = t1;
    ret *= t2;
    return ret;
}
