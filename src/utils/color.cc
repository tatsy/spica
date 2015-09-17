#define SPICA_COLOR_EXPORT
#include "color.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace spica {

    const Color Color::BLACK   = Color(0.0, 0.0, 0.0);
    const Color Color::WHITE   = Color(1.0, 1.0, 1.0);
    const Color Color::RED     = Color(1.0, 0.0, 0.0);
    const Color Color::GREEN   = Color(0.0, 1.0, 0.0);
    const Color Color::BLUE    = Color(0.0, 0.0, 1.0);
    const Color Color::CYAN    = Color(0.0, 1.0, 1.0);
    const Color Color::YELLOW  = Color(1.0, 1.0, 0.0);
    const Color Color::MAGENTA = Color(1.0, 0.0, 1.0);

    Color::Color()
        : _r(0.0)
        , _g(0.0)
        , _b(0.0) {
    }

    Color::Color(double red, double green, double blue)
        : _r(red)
        , _g(green)
        , _b(blue) {
    }

    Color::Color(const Color& color)
        : _r(color._r)
        , _g(color._g)
        , _b(color._b) {
    }

    Color::~Color() {
    }

    Color& Color::operator=(const Color& color) {
        this->_r = color._r;
        this->_g = color._g;
        this->_b = color._b;
        return *this;
    }

    Color& Color::operator+=(double x) {
        this->_r += x;
        this->_g += x;
        this->_b += x;
        return *this;
    }

    Color& Color::operator+=(const Color& c) {
        this->_r += c._r;
        this->_g += c._g;
        this->_b += c._b;
        return *this;
    }

    Color& Color::operator-=(const Color& c) {
        this->_r -= c._r;
        this->_g -= c._g;
        this->_b -= c._b;
        return *this;
    }

    Color& Color::operator-=(double x) {
        this->_r -= x;
        this->_g -= x;
        this->_b -= x;
        return *this;
    }

    Color& Color::operator*=(const Color& c) {
        this->_r *= c._r;
        this->_g *= c._g;
        this->_b *= c._b;
        return *this;
    }

    Color& Color::operator*=(double s) {
        this->_r *= s;
        this->_g *= s;
        this->_b *= s;
        return *this;
    }

    Color& Color::operator/=(const Color& c) {
        Assertion(c._r != 0.0 && c._g != 0.0 && c._b != 0.0,
                  "Zero division!!");
        this->_r /= c._r;
        this->_g /= c._g;
        this->_b /= c._b;
        return *this;
    }

    Color& Color::operator/=(double s) {
        Assertion(s != 0.0, "Zero division !!");
        this->_r /= s;
        this->_g /= s;
        this->_b /= s;
        return *this;
    }

    Color Color::operator-() const {
        return Color(-this->_r, -this->_g, -this->_b);
    }

    double Color::dot(const Color& color) const {
        return _r * color._r + _g * color._g + _b * color._b;
    }

    double Color::norm() const {
        return ::sqrt(this->squaredNorm());
    }

    double Color::squaredNorm() const {
        return this->dot(*this);
    }

    double Color::luminance() const {
        return 0.2126 * _r + 0.7152 * _g + 0.0722 * _b;
    }
    
    Color Color::sqrt(const Color& c) {
        using ::sqrt;
        Assertion(c._r >= 0.0 && c._g >= 0.0 && c._b >= 0.0,
                  "Specified vector has negative entries !!");
        return Color(sqrt(c._r), sqrt(c._g), sqrt(c._b));
    }

    Color Color::exp(const Color& c) {
        using ::exp;
        return Color(exp(c._r), exp(c._g), exp(c._b));
    }

    Color Color::maximum(const Color& c1, const Color& c2) {
        const double r = std::max(c1._r, c2._r);
        const double g = std::max(c1._g, c2._g);
        const double b = std::max(c1._b, c2._b);
        return Color(r, g, b);
    }

    Color Color::minimum(const Color& c1, const Color& c2) {
        const double r = std::min(c1._r, c2._r);
        const double g = std::min(c1._g, c2._g);
        const double b = std::min(c1._b, c2._b);
        return Color(r, g, b);
    }

    Color Color::clamp(const Color& lo, const Color& hi) const {
        return Color::maximum(lo, Color::minimum(*this, hi));
    }

    std::string Color::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << _r << ", " << _g << ", " << _b << ")";
        return ss.str();
    }

}  // namespace spica

spica::Color operator+(const spica::Color& c1, const spica::Color& c2) {
    spica::Color ret = c1;
    ret += c2;
    return ret;
}

spica::Color operator+(const spica::Color& c, double x) {
    spica::Color ret = c;
    ret += x;
    return ret;
}

spica::Color operator+(double x, const spica::Color& c) {
    spica::Color ret = c;
    ret += x;
    return ret;    
}

spica::Color operator-(const spica::Color& c1, const spica::Color& c2) {
    spica::Color ret = c1;
    ret -= c2;
    return ret;
}

spica::Color operator-(const spica::Color& c, double x) {
    spica::Color ret = c;
    ret -= x;
    return ret;
}

spica::Color operator-(double x, const spica::Color& c) {
    const double r = x - c.red();
    const double g = x - c.green();
    const double b = x - c.blue();
    return spica::Color(r, g, b);
}

spica::Color operator*(const spica::Color& c1, const spica::Color& c2) {
    spica::Color ret = c1;
    ret *= c2;
    return ret;
}

spica::Color operator*(const spica::Color& c, double s) {
    spica::Color ret = c;
    ret *= s;
    return ret;
}

spica::Color operator*(double s, const spica::Color& c) {
    spica::Color ret = c;
    ret *= s;
    return ret;
}

spica::Color operator/(const spica::Color& c1, const spica::Color& c2) {
    spica::Color ret = c1;
    ret /= c2;
    return ret;
}

spica::Color operator/(const spica::Color& c, double s) {
    spica::Color ret = c;
    ret /= s;
    return ret;
}

std::ostream& operator<<(std::ostream& os, const spica::Color& c) {
    os << c.toString();
    return os;
}