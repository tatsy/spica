#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MICROSURFACE_H_
#define _SPICA_MICROSURFACE_H_

#include "core/core.hpp"
#include "core/bxdf.h"

namespace spica {

// ----------------------------------------------------------------------------
// Microsurface height distribution
// ----------------------------------------------------------------------------

//! Microsurface height distribution interface
class MicrosurfaceHeight {
public:
    //! Height PDF
    virtual double P1(double h) const = 0;
    //! Height CDF
    virtual double C1(double h) const = 0;
    //! Inverse of the height CDF
    virtual double invC1(double U) const = 0;
};

/**
 * Uniform height distribution in [-1, 1]
 */
class MicrosurfaceUniformHeight : public MicrosurfaceHeight {
public:
    //! Height PDF
    double P1(double h) const override;
    //! Height CDF
    double C1(double h) const override;
    //! Inverse of the height CDF
    double invC1(double U) const override;
};

/**
 * Gaussian height distribution N(0, 1)
 */
class MicrosurfaceGaussianHeight : public MicrosurfaceHeight {
public:
    //! Height PDF
    double P1(double h) const override;
    //! Height CDF
    double C1(double h) const override;
    //! Inverse of the height CDF
    double invC1(double U) const override;
};


// ----------------------------------------------------------------------------
// Microsurface slope distribution
// ----------------------------------------------------------------------------

//! Microsurface slope distribution interface
class MicrosurfaceSlope {
public:
    MicrosurfaceSlope(double alphax, double alphay);

    //! Microfacet normal distribution
    double D(const Vector3d &wm) const;
    //! Visible normal distribution
    double D_wi(const Vector3d &wi, const Vector3d &wm) const;
    //! Sample visible normals
    Vector3d sampleD_wi(const Vector3d &wi, const Point2d &U) const;
    //! Projected area towards incident direction
    virtual double projectedArea(const Vector3d &wi) const = 0;

    //! Slope distribution
    virtual double P22(double slopex, double slopey) const = 0;
    //! Smith's lambda function
    virtual double lambda(const Vector3d &wi) const = 0;
    //! Sample visible slopes with alphax = alphay = 1.0
    virtual Vector2d sampleP22_11(double cosTheta, double U1, double U2) const = 0;

protected:
    //! Utility method to compute alpha_i
    double alpha_i(const Vector3d &wi) const;

    // Private parameters
    double alphax_, alphay_;
};

//! Beckmann slope distribution
class MicrosurfaceBeckmannSlope : public MicrosurfaceSlope {
public:
    MicrosurfaceBeckmannSlope(double alphax, double alphay);
    //! Projected area towards incident direction
    double projectedArea(const Vector3d &wi) const;

    //! Slope distribution
    double P22(double slopex, double slopey) const override;
    //! Smith's lambda function
    double lambda(const Vector3d &wi) const override;
    //! Sample visible slopes with alphax = alphay = 1.0
    Vector2d sampleP22_11(double cosTheta, double U1, double U2) const override;
};

//! Trowbridge-Reitz slope distribution
class MicrosurfaceTrowbridgeReitzSlope : public MicrosurfaceSlope {
public:
    MicrosurfaceTrowbridgeReitzSlope(double alphax, double alphay);
    //! Projected area towards incident direction
    double projectedArea(const Vector3d &wi) const;

    //! Slope distribution
    double P22(double slopex, double slopey) const override;
    //! Smith's lambda function
    double lambda(const Vector3d &wi) const override;
    //! Sample visible slopes with alphax = alphay = 1.0
    Vector2d sampleP22_11(double cosTheta, double U1, double U2) const override;
};


// ----------------------------------------------------------------------------
// Microsurface distribution
// ----------------------------------------------------------------------------

//! Microsurface distribution interface
class MicrosurfaceDistribution : public BxDF {
public:
    MicrosurfaceDistribution(BxDFType type,
                             MicrosurfaceHeight *hDist, MicrosurfaceSlope *sDist,
                             int scatteringOrder = 4);

protected:
    //! Masking function
    double G1(const Vector3d &wi) const;
    //! Masking function at height = h0
    double G1(const Vector3d &wi, double h0) const;
    //! Sample height in outgoing direction
    double sampleHeight(const Vector3d &wr, double hr, double U) const;

protected:
    //! Sample wo with a random walk
    virtual Vector3d sampleWi(const Vector3d &wo) const;
    //! Evaluate BSDF with a random wakl
    virtual Spectrum eval(const Vector3d &wo, const Vector3d &wi) const = 0;
    //! Evaluate local phase function
    virtual Spectrum evalPhaseFunction(const Vector3d &wo, const Vector3d &wi) const = 0;
    //! Sample local phase function
    virtual Vector3d samplePhaseFunction(const Vector3d &wi) const = 0;
    //! Compute MIS weight
    virtual double misWeight(const Vector3d &wo, const Vector3d &wi) const = 0;

    // Protected parameters
    const MicrosurfaceHeight *hDist_;
    const MicrosurfaceSlope *sDist_;
    const int scatteringOrder_;
};

//! Microsurface reflection
class MicrosurfaceReflection : public MicrosurfaceDistribution {
public:
    MicrosurfaceReflection(const Spectrum &re, Fresnel *fresnel,
                           MicrosurfaceHeight *hDist,
                           MicrosurfaceSlope *sDist, int scatteringOrder = 4);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;
    Spectrum sample(const Vector3d& wo, Vector3d* wi,
                    const Point2d& rands, double* pdf,
                    BxDFType* sampledType = nullptr) const override;
    //! Evaluate sample PDF
    double pdf(const Vector3d &wo, const Vector3d &wi) const override;

protected:
    Spectrum eval(const Vector3d &wo, const Vector3d &wi) const override;
    //! Evaluate local phase function
    Spectrum evalPhaseFunction(const Vector3d &wo, const Vector3d &wi) const override;
    //! Sample local phase function
    Vector3d samplePhaseFunction(const Vector3d &wi) const override;
    //! Compute MIS weight
    double misWeight(const Vector3d &wo, const Vector3d &wi) const override;

private:
    // Private parameters
    Spectrum re_;
    Fresnel *fresnel_;
};

//! Microsurface Fresnel reflection/transmission
class MicrosurfaceFresnel : public MicrosurfaceDistribution {
public:
    MicrosurfaceFresnel(const Spectrum &re, const Spectrum &tr, double etaA, double etaB,
                           MicrosurfaceHeight *hDist, MicrosurfaceSlope *sDist, int scatteringOrder = 4);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;
    Spectrum sample(const Vector3d& wo, Vector3d* wi,
                    const Point2d& rands, double* pdf,
                    BxDFType* sampledType = nullptr) const override;
    virtual double pdf(const Vector3d &wo, const Vector3d &wi) const override;

protected:
    //! Sample wo with a random walk
    Vector3d sampleWi(const Vector3d &wo) const override;
    //! Evaluate BSDF with a random wakl
    Spectrum eval(const Vector3d &wo, const Vector3d &wi) const override;
    //! Evaluate local phase function
    Spectrum evalPhaseFunction(const Vector3d &wo, const Vector3d &wi) const override;
    Spectrum evalPhaseFunction(const Vector3d &wo, const Vector3d &wi, bool woOutside, bool wiOutside) const;
    //! Sample local phase function
    Vector3d samplePhaseFunction(const Vector3d &wi) const override;
    Vector3d samplePhaseFunction(const Vector3d &wi, bool woOutside, bool *wiOutside) const;
    //! Compute MIS weight
    double misWeight(const Vector3d &wo, const Vector3d &wi) const override;

private:
    Spectrum re_, tr_;
    double etaA_, etaB_;
    std::unique_ptr<FresnelDielectric> fresnel_;
};

}  //namespace spica

#endif  // _SPICA_MICROSURFACE_H_
