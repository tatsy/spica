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

    CatmullRom& operator=(const CatmullRom&) = default;
    
    inline double x(int i) const { 
        Assertion(i >= 0 && i < size(), "Index out of bounds!!");
        return xs_[i];
    }

    inline double f(int i) const {
        Assertion(i >= 0 && i < size(), "Index out of bounds!!");
        return fs_[i];    
    }

    double evaluate(double x) const;
    double sample(double rand, double* fval, double* pdf) const;
    bool weights(double x, double* w0, double* w1, double* w2,
                 double* w3, int* offset = nullptr) const;

    inline int size() const { return static_cast<int>(fs_.size()); }

private:
    // Private fields
    std::vector<double> xs_;
    std::vector<double> fs_;
    std::vector<double> Fs_;
};

class SPICA_EXPORTS CatmullRom2D {
public:
    // Public methods
    CatmullRom2D(const std::vector<std::vector<double>>& fs,
                 const std::vector<double>& xs,
                 const std::vector<double>& ys,
                 const std::vector<double>& xNodes,
                 const std::vector<double>& yNodes);

    double evaluate(double x, double y) const;
    double sample(double rand1, double rand2, double* fval = nullptr,
                  double* pdf = nullptr) const;

    inline const CatmullRom& catmullRomX() const { return crx_; }
    inline const CatmullRom& catmullRomY() const { return cry_; }
    inline const std::vector<double>& merginalY() const { return merginalY_; }

private:
    std::vector<std::vector<double>> fs_;
    std::vector<std::vector<double>> Fs_;
    std::vector<double> merginalY_;
    CatmullRom crx_;
    CatmullRom cry_;
};

}  // namespace spica

#endif  // _SPICA_INTERPOLATION_H_
