#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_INTERPOLATION_H_
#define _SPICA_INTERPOLATION_H_

#include <vector>

#include "../core/common.h"

namespace spica {

class SPICA_EXPORTS CatmullRom {
public:
    // Public methods
    CatmullRom();
    CatmullRom(const std::vector<double>& fs,
               const std::vector<double>& xs);
    CatmullRom(const CatmullRom&) = default;
    CatmullRom(CatmullRom&& cr);

    CatmullRom& operator=(const CatmullRom&) = default;
    CatmullRom& operator=(CatmullRom&& cr);
    
    inline double x(int i) const { 
        Assertion(i >= 0 && i < size(), "Index out of bounds!!");
        return xs_[i];
    }

    inline double f(int i) const {
        Assertion(i >= 0 && i < size(), "Index out of bounds!!: %d for %ld", i, fs_.size());
        return fs_[i];
    }

    double evaluate(double x) const;
    double sample(double rand, double* fval = nullptr,
                  double* pdf = nullptr) const;

    inline int size() const { return static_cast<int>(fs_.size()); }

private:
    // Private methods
    bool weights(double x, double* w0, double* w1, double* w2,
                 double* w3, int* offset = nullptr) const;
 
    // Private fields
    std::vector<double> xs_;
    std::vector<double> fs_;
    std::vector<double> Fs_;
};

class SPICA_EXPORTS CatmullRom2D {
public:
    // Public methods
    CatmullRom2D();
    CatmullRom2D(const std::vector<std::vector<double>>& fs,
                 const std::vector<double>& xs,
                 const std::vector<double>& ys);
    CatmullRom2D(const CatmullRom2D&) = default;
    CatmullRom2D(CatmullRom2D&&);

    CatmullRom2D& operator=(const CatmullRom2D&) = default;
    CatmullRom2D& operator=(CatmullRom2D&&);
    inline double operator()(int i, int j) const {
        return fs_[i][j];
    }

    double evaluate(double x, double y) const;
    double sample(double rand1, double rand2, double* fval = nullptr,
                  double* pdf = nullptr) const;

    inline const std::vector<double>& xs() const { return xs_; }
    inline const std::vector<double>& ys() const { return ys_; }
    inline const std::vector<double>& marginalY() const { return marginalY_; }

private:
    std::vector<std::vector<double>> fs_;
    std::vector<std::vector<double>> cdf_;
    std::vector<double> marginalY_;
    std::vector<double> xs_;
    std::vector<double> ys_;
};

// -----------------------------------------------------------------------------
// Interpolation utility functions
// -----------------------------------------------------------------------------

SPICA_EXPORTS
bool catmullRomWeight(const std::vector<double>& nodes, double x,
                      double* w0, double* w1, double* w2, double* w3,
                      int* offset);

SPICA_EXPORTS
double integrateCatmullRom(const std::vector<double>& xs,
                           const std::vector<double>& fs,
                           std::vector<double>* cdf = nullptr);

}  // namespace spica

#endif  // _SPICA_INTERPOLATION_H_
