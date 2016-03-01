#define SPICA_API_EXPORT
#include "interpolation.h"

#include <algorithm>

namespace spica {

// -----------------------------------------------------------------------------
// Interpolation utility function definitions
// -----------------------------------------------------------------------------

namespace {

template <class Pred>
int findInterval(int lower, int upper, const Pred& pred) {
    int lo = lower;
    int hi = upper;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (pred(mid)) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return clamp(lo - 1, lower, upper - 2);
}

double integrateCatmullRom(const std::vector<double>& xs,
                           const std::vector<double>& fs,
                           std::vector<double>* cdf) {
    const int size = static_cast<int>(xs.size());
    cdf->resize(size);
    (*cdf)[0] = 0.0;
    double sum = 0.0;
    for (int i = 0; i < size - 1; i++) {
        const double x0 = xs[i];
        const double x1 = xs[i + 1];
        const double f0 = fs[i];
        const double f1 = fs[i + 1];
        const double dx = x1 - x0;

        double df0, df1;
        if (i > 0) {
            df0 = dx * (f1 - fs[i - 1]) / (x1 - xs[i - 1]);
        } else {
            df0 = f1 - f0;
        }

        if (i < size - 2) {
            df1 = dx * (fs[i + 2] - f0) / (xs[i +2] - x0);
        } else {
            df1 = f1 - f0;
        }

        sum += ((df0 - df1) / 12.0 + (f0 + f1) * 0.5) * dx;
        (*cdf)[i + 1] = sum;
    }
    return sum;
}

}  // anonymous namespace

bool catmullRomWeight(const std::vector<double>& nodes, double x,
                        double* w0, double* w1, double* w2, double* w3,
                        int* offset) {
    const int size = static_cast<int>(nodes.size());
    if (x < nodes[0] || nodes[size - 1] < x) return false;

    const int idx = findInterval(0, size, [&](int i) { return nodes[i] <= x; });
    if (offset) *offset = idx - 1;

    const double x0 = nodes[idx];
    const double x1 = nodes[idx + 1];

    const double t  = (x - x0) / (x1 - x0);
    const double t2 = t * t;
    const double t3 = t * t2;

    *w1 =  2.0 * t3 - 3.0 * t2 + 1.0;
    *w2 = -2.0 * t3 + 3.0 * t2;

    if (idx > 0) {
        const double tmp = (t3 - 2.0 * t2 + t) * (x1 - x0) / 
                           (x1 - nodes[idx - 1]);
        *w0 = -tmp;
        *w2 += tmp;
    } else {
        const double tmp = t3 - 2.0 * t2 + t;
        *w0 = 0.0;
        *w1 -= tmp;
        *w2 += tmp;
    }

    if (idx < size - 2) {
        const double tmp = (t3 - t2) * (x1 - x0) / (nodes[idx + 2] - x0);
        *w1 -= tmp;
        *w3 = tmp;
    } else {
        const double tmp = t3 - t2;
        *w1 -= tmp;
        *w2 += tmp;
        *w3 = 0.0;
    }
    return true;   
}

// -----------------------------------------------------------------------------
// CatmullRom method definitions
// -----------------------------------------------------------------------------

CatmullRom::CatmullRom()
    : xs_{}
    , fs_{}
    , Fs_{} {
}

CatmullRom::CatmullRom(const std::vector<double>& fs,
                       const std::vector<double>& xs)
    : xs_{ xs }
    , fs_{ fs }
    , Fs_{} {
    if (xs_.empty()) {
        xs_.resize(fs_.size());
        for (int i = 0; i < fs_.size(); i++) {
            xs_[i] = i;
        }
    }

    Fs_.resize(fs_.size());
    Fs_[0] = fs_[0];
    for (int i = 1; i < fs_.size(); i++) {
        Fs_[i] = Fs_[i - 1] + fs_[i];
    }
}

CatmullRom::CatmullRom(CatmullRom&& cr)
    : CatmullRom{} {
    this->operator=(std::move(cr));
}

CatmullRom& CatmullRom::operator=(CatmullRom&& cr) {
    this->xs_ = std::move(cr.xs_);
    this->fs_ = std::move(cr.fs_);
    this->Fs_ = std::move(cr.Fs_);
    return *this;
}

double CatmullRom::evaluate(double x) const {
    if (x < xs_[0] || xs_[this->size() - 1] < x) return 0.0;

    const int idx = findInterval(0, size(), [&](int i){ return xs_[i] <= x; });

    const double x0 = xs_[idx];
    const double x1 = xs_[idx + 1];
    const double f0 = fs_[idx];
    const double f1 = fs_[idx + 1];
    const double dx = x1 - x0;

    // Compute derivatives
    double df0, df1;
    if (idx > 0) {
        df0 = dx * (f1 - fs_[idx - 1]) / (x1 - xs_[idx - 1]);
    } else {
        df0 = f1 - f0;
    }

    if (idx < xs_.size() - 2) {
        df1 = dx * (fs_[idx + 2] - f0) / (xs_[idx + 2] - x0);
    } else {
        df1 = f1 - f0;
    }

    // Compute parameter and its powers
    const double t  = (x - x0) / (x1 - x0);
    const double t2 = t * t;
    const double t3 = t * t2;

    return (2.0 * t3 - 3.0 * t2 + 1.0) * f0 +
           (-2.0 * t3 + 3.0 * t2) * f1 + 
           (t3 - 2.0 * t2 + t) * df0 + (t3 - t2) * df1;
}

double CatmullRom::sample(double rand, double* fval, double* pdf) const {
    const double f = rand * Fs_[this->size() - 1];
    const int idx = findInterval(0, size(), [&](int i) { return Fs_[i] <= f; });

    const double x0 = xs_[idx];
    const double x1 = xs_[idx + 1];
    const double f0 = fs_[idx];
    const double f1 = fs_[idx + 1];
    const double dx = x1 - x0;

    double df0, df1;
    if (idx > 0) {
        df0 = dx * (f1 - fs_[idx - 1]) / (x1 - xs_[idx - 1]);
    } else {
        df0 = f1 - f0;
    }

    if (idx < this->size() - 2) {
        df1 = dx * (fs_[idx + 2] - f0) / (xs_[idx + 2] - x0);        
    } else {
        df1 = f1 - f0;
    }

    const double ft = (f - Fs_[idx]) / dx;
    double t;
    if (df0 != df1) {
        t = (f0 - std::sqrt(std::max(0.0, f0 * f0 + 2.0 * ft * (f1 - f0)))) / 
            (f0 - f1); 
    } else {
        t = ft / f0;
    }

    double lo = 0.0, hi = 1.0;
    double fhat;
    for (;;) {
        if (t < lo || hi < t) t = 0.5 * (lo + hi);

        const double t2 = t * t;
        const double t3 = t * t2;
        const double t4 = t * t3;
        fhat = f0 + df0 * t + (-2.0 * df0 - df1 + 3.0 * (f1 - f0)) * t2 +
               (df0 + df1 + 2.0 * (f0 - f1)) * t3;
        const double Fhat = f0 * t + 0.5 * df0 * t2 + 
               (1.0 / 3.0) * (-2.0 * df0 - df1 + 3.0 * (f1 - f0)) * t3 +
               0.25  *(df0 + df1 + 2.0 * (f0 - f1)) * t4;

        if (std::abs(Fhat - ft) < 1.0e-6 || hi - lo < 1.0e-6) break;

        if (Fhat - ft < 0.0) {
            lo = t;
        } else {
            hi = t;
        }

        t -= (Fhat - ft) / fhat;
    }

    if (fval) *fval = fhat;
    if (pdf)  *pdf = fhat / Fs_[Fs_.size() - 1];
    return x0 + dx * t;
}

bool CatmullRom::weights(double x, double* w0, double* w1, double* w2,
                         double* w3, int* offset) const {
    return catmullRomWeight(xs_, x, w0, w1, w2, w3, offset);
}

// -----------------------------------------------------------------------------
// CatmullRom2D method definitions
// -----------------------------------------------------------------------------

CatmullRom2D::CatmullRom2D()
    : fs_{}
    , cdf_{}
    , marginalY_{}
    , xs_{}
    , ys_{} {
}

CatmullRom2D::CatmullRom2D(const std::vector<std::vector<double>>& fs,
                           const std::vector<double>& xs,
                           const std::vector<double>& ys)
    : fs_{ fs }
    , cdf_{}
    , marginalY_{}
    , xs_{ xs }
    , ys_{ ys } {
    cdf_.resize(xs.size());
    marginalY_.resize(xs.size());
    for (int i = 0; i < xs.size(); i++) {
        marginalY_[i] = integrateCatmullRom(ys_, fs_[i], &cdf_[i]);
    }
}

CatmullRom2D::CatmullRom2D(CatmullRom2D&& cr)
    : CatmullRom2D{} {
    this->operator=(std::move(cr));
}

CatmullRom2D& CatmullRom2D::operator=(CatmullRom2D&& cr) {
    this->fs_ = std::move(cr.fs_);
    this->cdf_ = std::move(cr.cdf_);
    this->marginalY_ = std::move(cr.marginalY_);
    this->xs_ = std::move(cr.xs_);
    this->ys_ = std::move(cr.ys_);
    return *this;

}

double CatmullRom2D::evaluate(double x, double y) const {
    // Determine offset and weights for the first parameter
    int offx;
    double wx[4];
    if (!catmullRomWeight(xs_, x, &wx[0], &wx[1], &wx[2], &wx[3], &offx)) {
        return 0.0;
    }

    auto interpolate =
        [&](const std::vector<std::vector<double>>& array, int idx) {
        double value = 0.0;
        for (int i = 0; i < 4; i++) {
            if (wx[i] != 0.0) {
                value += array[offx + i][idx] * wx[i];
            }
        }
        return value;
    };

    // Determine offset and weights for the second parameter
    int offy;
    double wy[4];
    if (!catmullRomWeight(ys_, y, &wy[0], &wy[1], &wy[2], &wy[3], &offy)) {
        return 0.0;
    }

    // Compute return value
    double ret = 0.0;
    for (int i = 0; i < 4; i++) {
        if (wy[i] != 0.0) {
            ret += interpolate(fs_, offy + i) * wy[i];
        }
    }

    return ret;
}

double CatmullRom2D::sample(double rand1, double rand2, double* fval,
                            double* pdf) const {
    // Determine offset and weights for the first parameter
    int offset;
    double w[4];
    if (!catmullRomWeight(xs_, rand1, &w[0], &w[1], &w[2], &w[3], &offset)) {
        return 0.0;
    }

    auto interpolate =
        [&](const std::vector<std::vector<double>>& array, int idx) {
        double value = 0.0;
        for (int i = 0; i < 4; i++) {
            if (w[i] != 0.0) {
                value += array[offset + i][idx] * w[i];
            }
        }
        return value;
    };

    const double maximum = interpolate(cdf_, ys_.size() - 1);
    double u = rand2 * maximum;
    const int idx = findInterval(0, ys_.size(),
                                 [&](int i) { return interpolate(cdf_, i) <= u; });

    const double f0 = interpolate(fs_, idx);
    const double f1 = interpolate(fs_, idx + 1);
    const double x0 = ys_[idx];
    const double x1 = ys_[idx + 1];
    const double dx = x1 - x0;

    // Compute derivatives
    double df0, df1;
    if (idx > 0) {
        df0 = dx * (f1 - interpolate(fs_, idx - 1)) / (x1 - ys_[idx - 1]);
    } else {
        df0 = f1 - f0;
    }

    if (idx < ys_.size() - 2) {
        df1 = dx * (interpolate(fs_, idx + 2) - f0) / (ys_[idx + 2] - x0);        
    } else {
        df1 = f1 - f0;
    }

    u = (u - interpolate(cdf_, idx)) / dx;
    double t;
    if (f0 != f1) {
        t = (f0 - std::sqrt(std::max(0.0, f0 * f0 + 2.0 * u * (f1 - f0)))) / 
            (f0 - f1); 
    } else {
        t = u / f0;
    }

    double lo = 0.0, hi = 1.0;
    double fhat;
    for (;;) {
        if (t < lo || hi < t) t = 0.5 * (lo + hi);

        const double t2 = t * t;
        const double t3 = t * t2;
        const double t4 = t * t3;
        fhat = f0 + df0 * t + (-2.0 * df0 - df1 + 3.0 * (f1 - f0)) * t2 +
               (df0 + df1 + 2.0 * (f0 - f1)) * t3;
        const double Fhat = f0 * t + 0.5 * df0 * t2 + 
               (1.0 / 3.0) * (-2.0 * df0 - df1 + 3.0 * (f1 - f0)) * t3 +
               0.25  * (df0 + df1 + 2.0 * (f0 - f1)) * t4;

        if (std::abs(Fhat - u) < 1.0e-6 || hi - lo < 1.0e-6) break;

        if (Fhat - u < 0.0) {
            lo = t;
        } else {
            hi = t;
        }

        t -= (Fhat - u) / fhat;
    }

    if (fval) *fval = fhat;
    if (pdf)  *pdf = fhat / maximum;
    return x0 + dx * t;
}

}  // namespace spica
