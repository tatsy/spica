#define SPICA_API_EXPORT
#include "quaternion.h"

#include <cmath>

#include "vector3d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"

namespace spica {

    Quaternion::Quaternion()
        : _x{0.0}
        , _y{0.0}
        , _z{0.0}
        , _w{0.0} {
    }

    Quaternion::Quaternion(double x, double y, double z, double w)
        : _x{x}
        , _y{y}
        , _z{z}
        , _w{w} {
    }

    Quaternion::Quaternion(const Vector3d& v)
        : Quaternion{v.x(), v.y(), v.z()} {
    }

    Quaternion::Quaternion(const Quaternion& q)
        : _x{q._x}
        , _y{q._y}
        , _z{q._z}
        , _w{q._w} {
    }

    Quaternion::~Quaternion() {
    }

    Quaternion& Quaternion::operator=(const Quaternion& q) {
        this->_x = q._x;
        this->_y = q._y;
        this->_z = q._z;
        this->_w = q._w;
        return *this;
    }

    Quaternion& Quaternion::operator*=(const Quaternion& q) {
        double newx = _w * q._x + _x * q._w + _y * q._z - _z * q._y;
        double newy = _w * q._y + _y * q._w + _z * q._x - _x * q._z;
        double newz = _w * q._z + _z * q._w + _x * q._y - _y * q._x;
        double neww = - _x * q._x - _y * q._y - _z * q._z + _w * q._w;
        this->_x = newx;
        this->_y = newy;
        this->_z = newz;
        this->_w = neww;
        return *this;
    }

    Quaternion Quaternion::rotation(const Vector3d& axis, double theta) {
        double c = cos(theta * 0.5);
        double s = sin(theta * 0.5);
        return { axis.x() * s, axis.y() * s, axis.z() * s, c };
    }

    Vector3d Quaternion::applyTo(const Vector3d& v) {
        Quaternion q{v};
        return ((*this) * q * (this->inverse())).toVector3d();
    }

    Point3d Quaternion::applyTo(const Point3d& p) {
        return Point3d(applyTo(Vector3d(p)));
    }

    Normal3d Quaternion::applyTo(const Normal3d& n) {
        return Normal3d(applyTo(Vector3d(n)));
    }

    double Quaternion::squaredNorm() const {
        return _x * _x + _y * _y + _z * _z + _w * _w;
    }

    double Quaternion::norm() const {
        return sqrt(this->squaredNorm());
    }

    Quaternion Quaternion::inverse() const {
        double sqnrm = this->squaredNorm();
        Assertion(sqnrm != 0.0, "Zero length quaternion was inversed!!");
        return { -_x / sqnrm, -_y / sqnrm, -_z / sqnrm, _w / sqnrm };
    }

    Vector3d Quaternion::toVector3d() const {
        return Vector3d(_x, _y, _z);
    }

    std::string Quaternion::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(5);
        ss << "(" << _x << ", " << _y << ", " << _z << ", " << _w << ")";
        return std::move(ss.str());        
    }

}  // namespace spica

spica::Quaternion operator*(const spica::Quaternion& q1,
                            const spica::Quaternion& q2) {
    spica::Quaternion ret = q1;
    ret *= q2;
    return ret;
}

std::ostream& operator<<(std::ostream& os, const spica::Quaternion& q) {
    os << q.toString();
    return os;
}
