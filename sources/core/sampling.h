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
        inline double integral() const { return integral_; }
        inline int    count()    const { return static_cast<int>(func_.size()); }

    private:
        // Private methods
        static int findInterval(const std::vector<double>& cdf, double v);

        // Private fields
        std::vector<double> func_, cdf_;
        double integral_;
    };

    class Distribution2D {
    public:
        Distribution2D();
        Distribution2D(const std::vector<double>& data, int width, int height);
        Distribution2D(const Distribution2D& d);
        virtual ~Distribution2D();

        Distribution2D& operator=(const Distribution2D& d);

        Point2D sample(const Point2D& rands, double* pdf) const;
        double pdf(const Point2D& p) const;

    private:
        std::vector<Distribution1D> pCond_;
        Distribution1D pMarg_;
    };

    SPICA_EXPORTS void sampleUniformHemisphere(const Normal& normal, Vector3D* direction, const Point2D& rands);

    namespace sampler {

        SPICA_EXPORTS void onSphere(const Sphere& sphere, Point* position, Normal* normal);

        SPICA_EXPORTS void onSphere(const Sphere& sphere, Point* position, Normal* normal, double r1, double r2);
    
        SPICA_EXPORTS void onDisk(const Disk& disk, Point* position, Normal* normal);

        SPICA_EXPORTS void onTriangle(const Triangle& tri, Point* position, Normal* normal);

        SPICA_EXPORTS void onTriangle(const Triangle& tri, Point* position, Normal* normal, double r1, double r2);

        SPICA_EXPORTS void onQuad(const Quad& quad, Point* position, Normal* normal);

        SPICA_EXPORTS void onQuad(const Quad& quad, Point* position, Normal* normal, double r1, double r2);

        SPICA_EXPORTS void on(const Shape* shape, Point* position, Normal* normal);

        SPICA_EXPORTS void on(const Shape* shape, Point* position, Normal* normal, double r1, double r2);

        SPICA_EXPORTS
        void poissonDisk(const std::vector<Triangle>& triangles,
                         const double minDist,
                         std::vector<Point>* points,
                         std::vector<Normal>* normals);

    }  // namespace sampler

}  // namespace spica

#endif  // SPICA_SAMPLING_H_
