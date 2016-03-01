#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SAMPLING_H_
#define _SPICA_SAMPLING_H_

#include <vector>

#include "common.h"
#include "../math/vector3d.h"
#include "../shape/spica_shape.h"
#include "../random/random.h"

namespace spica {

class Distribution1D {
public:
    Distribution1D();
    Distribution1D(const std::vector<double>& data);
    Distribution1D(const Distribution1D& d);
    virtual ~Distribution1D();

    Distribution1D& operator=(const Distribution1D& d);
    double operator()(int i) const;

    double sample(double rand, double* pdf, int* offset = nullptr) const;
    int    sampleDiscrete(double rand, double* pdf) const;
    inline double integral() const { return integral_; }
    inline int    count()    const { return static_cast<int>(func_.size()); }

private:
    // Private methods
    static int findInterval(const std::vector<double>& cdf, double v);

    // Private fields
    std::vector<double> func_, cdf_;
    double integral_;

};  // class Distribution1D

class Distribution2D {
public:
    Distribution2D();
    Distribution2D(const std::vector<double>& data, int width, int height);
    Distribution2D(const Distribution2D& d);
    virtual ~Distribution2D();

    Distribution2D& operator=(const Distribution2D& d);

    Point2d sample(const Point2d& rands, double* pdf) const;
    double pdf(const Point2d& p) const;

private:
    std::vector<Distribution1D> pCond_;
    Distribution1D pMarg_;

};  // class Distribution2D

SPICA_EXPORTS Point2d  sampleConcentricDisk(const Point2d& rands);
SPICA_EXPORTS Vector3d sampleUniformSphere(const Point2d& rands);
SPICA_EXPORTS Vector3d sampleCosineHemisphere(const Point2d& rands);
SPICA_EXPORTS inline double   cosineHemispherePdf(double cosTheta) { return cosTheta * INV_PI; }
SPICA_EXPORTS void sampleUniformHemisphere(const Normal3d& normal, Vector3d* direction, const Point2d& rands);

SPICA_EXPORTS
void samplePoissonDisk(const std::vector<Triangle>& triangles,
                        const double minDist,
                        std::vector<Point3d>* points,
                        std::vector<Normal3d>* normals);

}  // namespace spica

#endif  // SPICA_SAMPLING_H_
