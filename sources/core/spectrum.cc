#define SPICA_API_EXPORT
#include "spectrum.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace spica {

    RGBSpectrum::RGBSpectrum()
        : r_{ 0.0 }
        , g_{ 0.0 }
        , b_{ 0.0 } {
    }

    RGBSpectrum::RGBSpectrum(double l)
        : r_{ l }
        , g_{ l }
        , b_{ l } {
    }

    RGBSpectrum::RGBSpectrum(double red, double green, double blue)
        : r_{ red }
        , g_{ green }
        , b_{ blue } {
    }

    RGBSpectrum::RGBSpectrum(const RGBSpectrum& color)
        : r_{ color.r_ }
        , g_{ color.g_ }
        , b_{ color.b_ } {
    }

    RGBSpectrum::~RGBSpectrum() {
    }

    RGBSpectrum& RGBSpectrum::operator=(const RGBSpectrum& color) {
        this->r_ = color.r_;
        this->g_ = color.g_;
        this->b_ = color.b_;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator+=(double x) {
        this->r_ += x;
        this->g_ += x;
        this->b_ += x;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator+=(const RGBSpectrum& c) {
        this->r_ += c.r_;
        this->g_ += c.g_;
        this->b_ += c.b_;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator-=(const RGBSpectrum& c) {
        this->r_ -= c.r_;
        this->g_ -= c.g_;
        this->b_ -= c.b_;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator-=(double x) {
        this->r_ -= x;
        this->g_ -= x;
        this->b_ -= x;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator*=(const RGBSpectrum& c) {
        this->r_ *= c.r_;
        this->g_ *= c.g_;
        this->b_ *= c.b_;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator*=(double s) {
        this->r_ *= s;
        this->g_ *= s;
        this->b_ *= s;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator/=(const RGBSpectrum& c) {
        Assertion(c.r_ != 0.0 && c.g_ != 0.0 && c.b_ != 0.0,
                  "Zero division!!");
        this->r_ /= c.r_;
        this->g_ /= c.g_;
        this->b_ /= c.b_;
        return *this;
    }

    RGBSpectrum& RGBSpectrum::operator/=(double s) {
        Assertion(s != 0.0, "Zero division !!");
        this->r_ /= s;
        this->g_ /= s;
        this->b_ /= s;
        return *this;
    }

    RGBSpectrum RGBSpectrum::operator-() const {
        return RGBSpectrum(-this->r_, -this->g_, -this->b_);
    }

    bool RGBSpectrum::isBlack() const {
        return r_ == 0.0 && g_ == 0.0 && b_ == 0.0;
    }

    double RGBSpectrum::dot(const RGBSpectrum& color) const {
        return r_ * color.r_ + g_ * color.g_ + b_ * color.b_;
    }

    double RGBSpectrum::norm() const {
        return ::sqrt(this->squaredNorm());
    }

    double RGBSpectrum::squaredNorm() const {
        return this->dot(*this);
    }

    double RGBSpectrum::luminance() const {
        return 0.2126 * r_ + 0.7152 * g_ + 0.0722 * b_;
    }
    
    RGBSpectrum RGBSpectrum::sqrt(const RGBSpectrum& c) {
        using ::sqrt;
        Assertion(c.r_ >= 0.0 && c.g_ >= 0.0 && c.b_ >= 0.0,
                  "Specified vector has negative entries !!");
        return RGBSpectrum(sqrt(c.r_), sqrt(c.g_), sqrt(c.b_));
    }

    RGBSpectrum RGBSpectrum::exp(const RGBSpectrum& c) {
        using ::exp;
        return RGBSpectrum(exp(c.r_), exp(c.g_), exp(c.b_));
    }

    RGBSpectrum RGBSpectrum::log(const RGBSpectrum& c) {
        using ::log;
        return RGBSpectrum(log(c.r_), log(c.g_), log(c.b_));
    }

    RGBSpectrum RGBSpectrum::maximum(const RGBSpectrum& c1, const RGBSpectrum& c2) {
        const double r = std::max(c1.r_, c2.r_);
        const double g = std::max(c1.g_, c2.g_);
        const double b = std::max(c1.b_, c2.b_);
        return RGBSpectrum(r, g, b);
    }

    RGBSpectrum RGBSpectrum::minimum(const RGBSpectrum& c1, const RGBSpectrum& c2) {
        const double r = std::min(c1.r_, c2.r_);
        const double g = std::min(c1.g_, c2.g_);
        const double b = std::min(c1.b_, c2.b_);
        return RGBSpectrum(r, g, b);
    }

    RGBSpectrum RGBSpectrum::clamp(const RGBSpectrum& c, const RGBSpectrum& lo, const RGBSpectrum& hi) {
        return RGBSpectrum::maximum(lo, RGBSpectrum::minimum(c, hi));
    }

    std::string RGBSpectrum::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "(" << r_ << ", " << g_ << ", " << b_ << ")";
        return ss.str();
    }

}  // namespace spica

spica::RGBSpectrum operator+(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2) {
    spica::RGBSpectrum ret = c1;
    ret += c2;
    return ret;
}

spica::RGBSpectrum operator+(const spica::RGBSpectrum& c, double x) {
    spica::RGBSpectrum ret = c;
    ret += x;
    return ret;
}

spica::RGBSpectrum operator+(double x, const spica::RGBSpectrum& c) {
    spica::RGBSpectrum ret = c;
    ret += x;
    return ret;    
}

spica::RGBSpectrum operator-(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2) {
    spica::RGBSpectrum ret = c1;
    ret -= c2;
    return ret;
}

spica::RGBSpectrum operator-(const spica::RGBSpectrum& c, double x) {
    spica::RGBSpectrum ret = c;
    ret -= x;
    return ret;
}

spica::RGBSpectrum operator-(double x, const spica::RGBSpectrum& c) {
    const double r = x - c.red();
    const double g = x - c.green();
    const double b = x - c.blue();
    return spica::RGBSpectrum(r, g, b);
}

spica::RGBSpectrum operator*(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2) {
    spica::RGBSpectrum ret = c1;
    ret *= c2;
    return ret;
}

spica::RGBSpectrum operator*(const spica::RGBSpectrum& c, double s) {
    spica::RGBSpectrum ret = c;
    ret *= s;
    return ret;
}

spica::RGBSpectrum operator*(double s, const spica::RGBSpectrum& c) {
    spica::RGBSpectrum ret = c;
    ret *= s;
    return ret;
}

spica::RGBSpectrum operator/(const spica::RGBSpectrum& c1, const spica::RGBSpectrum& c2) {
    spica::RGBSpectrum ret = c1;
    ret /= c2;
    return ret;
}

spica::RGBSpectrum operator/(const spica::RGBSpectrum& c, double s) {
    spica::RGBSpectrum ret = c;
    ret /= s;
    return ret;
}

std::ostream& operator<<(std::ostream& os, const spica::RGBSpectrum& c) {
    os << c.toString();
    return os;
}