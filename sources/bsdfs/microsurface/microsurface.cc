#include "microsurface.h"

#include <ctime>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "core/common.h"
#include "core/math.h"
#include "core/vect_math.h"
#include "core/point2d.h"
#include "core/vector2d.h"
#include "core/vector3d.h"
#include "core/random.h"


namespace spica {

static thread_local Random random = Random((uint32_t)std::rand());

// ----------------------------------------------------------------------------
// Microsurface uniform height distribution
// ----------------------------------------------------------------------------

double MicrosurfaceUniformHeight::P1(double h) const {
    return (h >= -1.0 && h <= 1.0) ? 0.5 : 0.0;
}

double MicrosurfaceUniformHeight::C1(double h) const {
    return std::max(0.0, std::min(0.5 * (h + 1.0), 1.0));
}

double MicrosurfaceUniformHeight::invC1(double U) const {
    return std::max(-1.0, std::min(2.0 * U - 1.0, 1.0));
}

// ----------------------------------------------------------------------------
// Microsurface Gaussian height distribution
// ----------------------------------------------------------------------------

double MicrosurfaceGaussianHeight::P1(double h) const {
    static const double INV_SQRT_2_PI = 1.0 / std::sqrt(2.0 * PI);
    return INV_SQRT_2_PI * std::exp(-0.5 * h * h);
}

double MicrosurfaceGaussianHeight::C1(double h) const {
    static const double INV_SQRT_2 = 1.0 / std::sqrt(2.0);
    return 0.5 + 0.5 * math::erf(INV_SQRT_2 * h);
}

double MicrosurfaceGaussianHeight::invC1(double U) const {
    static const double SQRT_2 = std::sqrt(2.0);
    return SQRT_2 * math::erfinv(2.0 * U - 1.0);
}


// ----------------------------------------------------------------------------
// Microsurface slope distribution interface
// ----------------------------------------------------------------------------

MicrosurfaceSlope::MicrosurfaceSlope(double alphax, double alphay)
    : alphax_{alphax}
    , alphay_{alphay} {

    static const double minAlpha = 1.0e-5;
    if (alphax_ <= minAlpha) {
        Warning("alphax must be more than 0.0, it was automatically clamped to %f", minAlpha);
        alphax_ = minAlpha;
    }

    if (alphay_ <= minAlpha) {
        Warning("alphay must be more than 0.0, it was automatically clamped to %f", minAlpha);
        alphay_ = minAlpha;
    }
}

double MicrosurfaceSlope::D(const Vector3d &wm) const {
    if (vect::cosTheta(wm) <= 0.0) {
        return 0.0;
    }

    // Slope for wm
    const double slopex = -wm.x() / wm.z();
    const double slopey = -wm.y() / wm.z();

    // Variable transformation
    const double cos2Theta = vect::cos2Theta(wm);
    return P22(slopex, slopey) / (cos2Theta * cos2Theta);
}

double MicrosurfaceSlope::D_wi(const Vector3d &wi, const Vector3d &wm) const {
    if (vect::cosTheta(wm) <= 0.0) {
        return 0.0;
    }

    const double pArea = projectedArea(wi);
    if (pArea == 0.0) {
        return 0.0;
    }

    const double c = 1.0 / pArea;
    return c * std::max(0.0, vect::dot(wi, wm)) * D(wm);
}

Vector3d MicrosurfaceSlope::sampleD_wi(const Vector3d &wi, const Point2d &U) const {
    // 1. Stretch
    const Vector3d wiStretched = vect::normalize(Vector3d(wi.x() * alphax_, wi.y() * alphay_, wi.z()));

    // 2. Sample visible slope with alphax = alphay = 1.0
    Vector2d slope11 = sampleP22_11(vect::cosTheta(wiStretched), U[0], U[1]);

    // 3. Rotate
    const double phi = std::atan2(wiStretched.y(), wiStretched.x());
    const double cosPhi = std::cos(phi);
    const double sinPhi = std::sin(phi);
    double slopex = cosPhi * slope11.x() - sinPhi * slope11.y();
    double slopey = sinPhi * slope11.x() + cosPhi * slope11.y();

    // 4. Unstretch
    slopex *= alphax_;
    slopey *= alphay_;
    Assertion(!std::isinf(slopex) && !std::isnan(slopex), "Invalid slopex: %f\n", slopex);
    Assertion(!std::isinf(slopey) && !std::isnan(slopey), "Invalid slopex: %f\n", slopey);

    // 5. Convert to normal
    return vect::normalize(Vector3d(-slopex, -slopey, 1.0));
}

double MicrosurfaceSlope::alpha_i(const Vector3d &wi) const {
    const double cos2Phi = vect::cos2Phi(wi);
    const double sin2Phi = vect::sin2Phi(wi);
    return std::sqrt(cos2Phi * alphax_ * alphax_ + sin2Phi * alphay_ * alphay_);
}

double MicrosurfaceSlope::projectedArea(const Vector3d &wi) const {
    // [Heitz et al. 2016] Appendix B.
    const double cosTheta = vect::cosTheta(wi);
    return (1.0 + lambda(wi)) * cosTheta;
}

// ----------------------------------------------------------------------------
// Microsurface Beckmann slope distribution
// ----------------------------------------------------------------------------

MicrosurfaceBeckmannSlope::MicrosurfaceBeckmannSlope(double alphax, double alphay)
    : MicrosurfaceSlope{alphax, alphay} {
}

double MicrosurfaceBeckmannSlope::P22(double slopex, double slopey) const {
    const double alphax2 = alphax_ * alphax_;
    const double alphay2 = alphay_ * alphay_;
    const double phase = slopex * slopex / alphax2 + slopey * slopey / alphay2;
    return (1.0 / (PI * alphax_ * alphay_)) * std::exp(-phase);
}

double MicrosurfaceBeckmannSlope::lambda(const Vector3d &wi) const {
    const double cosTheta = vect::cosTheta(wi);
    if (cosTheta < -0.9999) {
        return -1.0;
    } else if (cosTheta > 0.9999) {
        return 0.0;
    }

    const double thetaI = std::acos(cosTheta);
    const double alpha = alpha_i(wi);
    const double a = 1.0 / (alpha * std::tan(thetaI));

    return 0.5 * (math::erf(a) - 1.0) + (1.0 / (2.0 * a * std::sqrt(PI)) * std::exp(-a * a));
}

Vector2d MicrosurfaceBeckmannSlope::sampleP22_11(double cosThetaI, double U1, double U2) const {
   /* Special case (normal incidence) */
    if (cosThetaI > 0.9999) {
        const double r = std::sqrt(-std::log(1.0 - U1));
        const double sinPhi = std::sin(2.0 * PI * U2);
        const double cosPhi = std::cos(2.0 * PI * U2);
        const double slopex = r * cosPhi;
        const double slopey = r * sinPhi;
        return Vector2d(slopex, slopey);
    }

    // Solve inverse of C22 with the simple binary search
    const double sinThetaI = std::sqrt(std::max(0.0, 1.0 - cosThetaI * cosThetaI));
    const double tanThetaI = sinThetaI / cosThetaI;
    const double cotThetaI = 1.0 / tanThetaI;

    double low = -1.0;
    double high = math::erf(cotThetaI);
    double samplex = std::max(U1, 1.0e-6);

    double thetaI = std::acos(cosThetaI);
    double fit = 1.0 + thetaI * (-0.876 + thetaI * (0.4265 - 0.0594 * thetaI));
    double mid = high - (1.0 + high) * std::pow(1.0 - samplex, fit);

    // Normalization factor below equals to (G1(w) / 2)^{-1}
    static const double SQRT_PI_INV = 1.0 / std::sqrt(PI);
    const double normalization = 1.0 / (1.0 + high + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

    for (int it = 0; it < 16; it++) {
        if (mid < low || high < mid) {
            mid = 0.5 * (low + high);
        }

        double ierf = math::erfinv(mid);
        double value = normalization * (1.0 + mid + SQRT_PI_INV * tanThetaI * std::exp(-ierf * ierf)) - samplex;
        double derivative = normalization * (1.0 - ierf * tanThetaI);

        if (std::abs(value) < 1.0e-6) break;

        if (value > 0.0) {
            high = mid;
        } else {
            low = mid;
        }

        mid -= value / derivative;
    }

    const double slopex = math::erfinv(mid);
    const double slopey = math::erfinv(2.0 * std::max(U2, 1.0e-6) - 1.0);
    return Vector2d(slopex, slopey);
}


// ----------------------------------------------------------------------------
// Microsurface TrowbridgeReitz slope distribution
// ----------------------------------------------------------------------------

MicrosurfaceTrowbridgeReitzSlope::MicrosurfaceTrowbridgeReitzSlope(double alphax, double alphay)
    : MicrosurfaceSlope{alphax, alphay} {
}

double MicrosurfaceTrowbridgeReitzSlope::P22(double slopex, double slopey) const {
    const double alphax2 = alphax_ * alphax_;
    const double alphay2 = alphay_ * alphay_;
    const double tmp = 1.0 + slopex * slopex / alphax2 + slopey * slopey / alphay2;
    return 1.0 / (PI * alphax_ * alphay_) / (tmp * tmp);
}

double MicrosurfaceTrowbridgeReitzSlope::lambda(const Vector3d &wi) const {
    const double cosTheta = vect::cosTheta(wi);
    if (cosTheta < -0.9999) {
        return -1.0;
    } else if (cosTheta > 0.9999) {
        return 0.0;
    }

    const double theta = std::acos(cosTheta);
    const double alpha = alpha_i(wi);
    const double a = 1.0 / (alpha * std::tan(theta));

    // TODO: this sign is necessary? (in PBRT v3, it is always positive)
    const double sign = a == 0.0 ? 0.0 : a / std::abs(a);
    return 0.5 * (-1.0 + sign * std::sqrt(1.0 + 1.0 / (a * a)));
}

Vector2d MicrosurfaceTrowbridgeReitzSlope::sampleP22_11(double cosTheta, double U1, double U2) const {
    if (cosTheta > 0.9999) {
        double r = std::sqrt(U1 / (1.0 - U1));
        double phi = 2.0 * PI * U2;
        const double slopex = r * std::cos(phi);
        const double slopey = r * std::sin(phi);
        return Vector2d(slopex, slopey);
    }

    double sinTheta = sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
    double tanTheta = sinTheta / cosTheta;
    double a = 1.0 / tanTheta;
    double G1 = 2.0 / (1.0 + std::sqrt(1.0 + 1.0 / (a * a)));

    double A = 2.0 * U1 / G1 - 1.0;
    double tmp = 1.0 / (A * A - 1.0);
    if (tmp > 1.0e10) tmp = 1.0e10;
    double B = tanTheta;
    double D = sqrt(std::max(0.0, B * B * tmp * tmp - (A * A - B * B) * tmp));
    double slopex1 = B * tmp - D;
    double slopex2 = B * tmp + D;
    const double slopex = (A < 0.0 || slopex2 > 1.0 / tanTheta) ? slopex1 : slopex2;
    Assertion(!std::isinf(slopex), "slopex is infinity.");
    Assertion(!std::isnan(slopex), "slopex is NaN.");

    double S, U;
    if (U2 > 0.5) {
        S = 1.0;
        U = 2.0 * (U2 - 0.5);
    } else {
        S = -1.0;
        U = 2.0 * (0.5 - U2);
    }

    double z = (U * (U * (U * 0.27385 - 0.73369) + 0.46341)) / 
               (U * (U * (U * 0.093073 + 0.309420) - 1.0) + 0.597999);
    const double slopey = S * z * std::sqrt(1.0 + (slopex) * (slopex));
    Assertion(!std::isinf(slopey), "slopey is infinity.");
    Assertion(!std::isnan(slopey), "slopey is NaN");

    return Vector2d(slopex, slopey);
}


// ----------------------------------------------------------------------------
// Microsurface distribution interface
// ----------------------------------------------------------------------------

MicrosurfaceDistribution::MicrosurfaceDistribution(BxDFType type,
                                                   MicrosurfaceHeight *hDist,
                                                   MicrosurfaceSlope *sDist,
                                                   int scatteringOrder)
    : BxDF{type}
    , hDist_{hDist}
    , sDist_{sDist}
    , scatteringOrder_{scatteringOrder} {
}

double MicrosurfaceDistribution::G1(const Vector3d &wi) const {
    if (wi.z() > 0.9999) {
        return 1.0;
    } else if (wi.z() <= 0.0) {
        return 0.0;
    }

    const double lambda = sDist_->lambda(wi);
    return 1.0 / (1.0 + lambda);
}

double MicrosurfaceDistribution::G1(const Vector3d &wi, double h0) const {
    if (wi.z() > 0.9999) {
        return 1.0;
    } else if (wi.z() <= 0.0) {
        return 0.0;
    }

    // [Heitz et al. 2016] Eq(1)
    const double C1_h0 = hDist_->C1(h0);
    const double lambda = sDist_->lambda(wi);
    return std::pow(C1_h0, lambda);
}

double MicrosurfaceDistribution::sampleHeight(const Vector3d &wr, double hr, double U) const {
    if (wr.z() > 0.9999) {
        return INFTY;
    } else if (wr.z() < -0.9999) {
        const double C1 = hDist_->C1(hr);
        return hDist_->invC1(U * C1);
    }

    if (std::abs(wr.z()) < 0.0001) {
        return hr;
    }

    // Probability of intersection
    const double g1 = G1(wr, hr);
    if (U > 1.0 - g1) {
        return INFTY;
    }

    const double C1 = hDist_->C1(hr);
    const double lambda = sDist_->lambda(wr);
    const double h = C1 / std::pow(1.0 - U, 1.0 / lambda);
    return h;
}

Vector3d MicrosurfaceDistribution::sampleWi(const Vector3d &wo) const {
    // Initialization
    Vector3d wr = -wo;
    double hr = 1.0 + hDist_->invC1(0.9999);

    // Random walk
    for (int r = 0; r < scatteringOrder_; r++) {
        // Next height
        const double U = random.get1D();
        hr = sampleHeight(wr, hr, U);

        // Leave the surface
        if (hr == INFTY) {
            break;
        }

        // Next direction
        wr = samplePhaseFunction(-wr);

        // Check validity
        if (std::isinf(hr) || std::isnan(hr) || std::isinf(wr.z()) || std::isnan(wr.z())) {
            return Vector3d(0.0, 0.0, 1.0);
        }
    }

    return wr;
}

// ----------------------------------------------------------------------------
// Microsurface reflection
// ----------------------------------------------------------------------------

MicrosurfaceReflection::MicrosurfaceReflection(const Spectrum &re,
                                               Fresnel *fresnel,
                                               MicrosurfaceHeight *hDist,
                                               MicrosurfaceSlope *sDist,
                                               int scatteringOrder)
    : MicrosurfaceDistribution{BxDFType::Reflection | BxDFType::Glossy, hDist, sDist, scatteringOrder}
    , re_{re}
    , fresnel_{fresnel} {
}

Spectrum MicrosurfaceReflection::f(const Vector3d &wo, const Vector3d &wi) const {
    const double cosThetaO = vect::cosTheta(wo);
    const double cosThetaI = vect::cosTheta(wi);
    if (cosThetaI == 0.0 || cosThetaO == 0.0) return Spectrum(0.0);

    const Vector3d wh = vect::normalize(wi + wo);
    const Spectrum er = eval(wo, wi);
    return re_ * er;
}

Spectrum MicrosurfaceReflection::sample(const Vector3d& wo, Vector3d* wi,
                                        const Point2d& rands, double* pdf,
                                        BxDFType* sampledType) const {
    if (wo.z() == 0.0) return Spectrum(0.0);

    *wi = sampleWi(wo);
    if (!vect::sameHemisphere(wo, *wi)) return Spectrum(0.0);

    *pdf = 1.0;
    return f(wo, *wi);
}

Spectrum MicrosurfaceReflection::eval(const Vector3d &wo, const Vector3d &wi) const {
    if (wi.z() < 0.0) {
        return Spectrum(0.0);
    }

    // Initialization
    Vector3d wr = -wo;
    double hr = 1.0 + hDist_->invC1(0.999);
    Spectrum er(1.0);

    // Random walk
    Spectrum sum(0.0);
    for (int r = 0; r < scatteringOrder_; r++) {
        // Next height
        const double U = random.get1D();
        hr = sampleHeight(wr, hr, U);

        // Leave the surface
        if (hr == INFTY) {
            break;
        }

        // Next direction
        Vector3d wrNext = samplePhaseFunction(-wr);
    
        // Update er
        const Vector3d wh = vect::normalize(wr + wrNext);
        Spectrum F = fresnel_->evaluate(vect::dot(wrNext, wh));
        er *= F;

        // Next event estimation
        double phase = evalPhaseFunction(-wr, wi);
        double shadowing = G1(wi, hr);

        Spectrum I = er * phase * shadowing;

        if (I.isValid()) {
            sum += I;
        }
        wr = wrNext;

        // Check validity
        if (std::isinf(hr) || std::isnan(hr) || std::isinf(wr.z()) || std::isnan(wr.z())) {
            return Spectrum(0.0);
        }
    }
    
    return sum;
}

double MicrosurfaceReflection::evalPhaseFunction(const Vector3d &wo, const Vector3d &wi) const {
    const Vector3d wh = vect::normalize(wi + wo);
    if (wh.z() < 0.0) {
        return 0.0;
    }

    // [Heitz et al. 2016] Eq(35)
    return 0.25 * sDist_->D_wi(wo, wh) / vect::dot(wo, wh);
}

Vector3d MicrosurfaceReflection::samplePhaseFunction(const Vector3d &wi) const {
    const Point2d U = random.get2D();
    const Vector3d wm = sDist_->sampleD_wi(wi, U);
    return vect::reflect(wi, wm);
}

// ----------------------------------------------------------------------------
// Microsurface scattering
// ----------------------------------------------------------------------------

MicrosurfaceScattering::MicrosurfaceScattering(const Spectrum &re, const Spectrum &tr, double etaA, double etaB,
                                               MicrosurfaceHeight *hDist, MicrosurfaceSlope *sDist, int scatteringOrder)
    : MicrosurfaceDistribution{BxDFType::Reflection | BxDFType::Transmission | BxDFType::Glossy, hDist, sDist, scatteringOrder}
    , re_{re}
    , tr_{tr}
    , etaA_{etaA}
    , etaB_{etaB} {
}

Spectrum MicrosurfaceScattering::f(const Vector3d& wo, const Vector3d& wi) const {
    return Spectrum(0.0);
}

Spectrum MicrosurfaceScattering::sample(const Vector3d& wo, Vector3d* wi,
                                        const Point2d& rands, double* pdf,
                                        BxDFType* sampledType) const {
    return Spectrum(0.0);
}

Spectrum MicrosurfaceScattering::eval(const Vector3d &wo, const Vector3d &wi) const {
    return Spectrum(0.0);
}

double MicrosurfaceScattering::evalPhaseFunction(const Vector3d &wo, const Vector3d &wi) const {
    return evalPhaseFunction(wo, wi, true, true) + evalPhaseFunction(wo, wi, true, false);
}

double MicrosurfaceScattering::evalPhaseFunction(const Vector3d &wo, const Vector3d &wi, bool woOutside, bool wiOutside) const {
    return 0.0;
}

Vector3d MicrosurfaceScattering::samplePhaseFunction(const Vector3d &wi) const {
    return Vector3d(0.0, 0.0, 1.0);
}

}  //namespace spica