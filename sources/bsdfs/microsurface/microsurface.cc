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

namespace {
static inline double  abgam (double x)
{
  double  gam[10],
          temp;

  gam[0] = 1./ 12.;
  gam[1] = 1./ 30.;
  gam[2] = 53./ 210.;
  gam[3] = 195./ 371.;
  gam[4] = 22999./ 22737.;
  gam[5] = 29944523./ 19733142.;
  gam[6] = 109535241009./ 48264275462.;
  temp = 0.5*log (2*PI) - x + (x - 0.5)*log (x)
    + gam[0]/(x + gam[1]/(x + gam[2]/(x + gam[3]/(x + gam[4] /
      (x + gam[5]/(x + gam[6]/x))))));

  return temp;
}

static inline double  gamma (double x)
{
  double  result;
  result = exp (abgam (x + 5))/(x*(x + 1)*(x + 2)*(x + 3)*(x + 4));
  return result;
}

static inline double  beta (double m, double n)
{
  return (gamma (m)*gamma (n)/gamma (m + n));
}
}

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
    if (wm.z() <= 0.0) {
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
    if (wm.z() <= 0.0) {
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

// ----------------------------------------------------------------------------
// Microsurface Beckmann slope distribution
// ----------------------------------------------------------------------------

MicrosurfaceBeckmannSlope::MicrosurfaceBeckmannSlope(double alphax, double alphay)
    : MicrosurfaceSlope{alphax, alphay} {
}

double MicrosurfaceBeckmannSlope::projectedArea(const Vector3d &wi) const {
    if (wi.z() > 0.9999) {
        return 1.0;
    } else if (wi.z() < -0.9999) {
        return 0.0;
    }

    // [Heitz et al. 2016] Appendix B.
    // The projected area can be represented as "(1.0 + Lambda) * cosTheta".
    // However, following representation is more numerically stable.
    const double alpha = alpha_i(wi);
    const double cosTheta = vect::cosTheta(wi);
    const double sinTheta = vect::sinTheta(wi);
    const double tanTheta = sinTheta / (cosTheta + EPS);
    const double a = 1.0 / (alpha * tanTheta);
    static const double INV_2_SQRT_PI = 1.0 / (2.0 * std::sqrt(PI));
    return 0.5 * cosTheta * (math::erf(a) + 1.0) + (alpha * sinTheta) * INV_2_SQRT_PI * std::exp(-a * a);
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

    static const double INV_2_SQRT_PI = 1.0 / (2.0 * std::sqrt(PI));
    double a = cotThetaI;
    const double pArea = 0.5 * (math::erf(a) + 1.0) * cosThetaI + INV_2_SQRT_PI * sinThetaI * std::exp(-a * a);
    if (pArea < 0.0001 || std::isinf(pArea) || std::isnan(pArea)) {
        return Vector2d(0.0, 0.0);
    }

    // VNDF normalization factor
    const double c = 1.0 / pArea;

    // Bisection method
    double erfMin = -0.9999;
    double erfMax = std::max(erfMin, math::erf(cotThetaI));
    double erfCurrent = 0.5 * (erfMin + erfMax);

    const double samplex = U1;
    while (erfMax - erfMin > 1.0e-5) {
        if (erfCurrent < erfMin || erfMax < erfCurrent) {
            erfCurrent = 0.5 * (erfMin + erfMax);
        }

        // Evaluate slope
        const double slope = math::erfinv(erfCurrent);

        // CDF
        const double CDF = (slope >= cotThetaI) ? 1.0 : c * (INV_2_SQRT_PI * sinThetaI * std::exp(-slope * slope) + cosThetaI * (0.5 + 0.5 * math::erf(slope)));
        const double diff = CDF - samplex;

        // Test
        if (std::abs(diff) < 1.0e-5) {
            break;
        }

        // Update bounds
        if (diff > 0.0) {
            erfMax = erfCurrent;
        } else {
            erfMin = erfCurrent;
        }

        // Update estimate
        const double derivative = 0.5 * c * cosThetaI - 0.5 * c * sinThetaI * slope;
        erfCurrent -= diff / derivative;
    }

    const double slopex = math::erfinv(std::min(erfMax, std::max(erfMin, erfCurrent)));
    const double slopey = math::erfinv(2.0 * std::max(U2, 1.0e-6) - 1.0);
    return Vector2d(slopex, slopey);
}


// ----------------------------------------------------------------------------
// Microsurface TrowbridgeReitz slope distribution
// ----------------------------------------------------------------------------

MicrosurfaceTrowbridgeReitzSlope::MicrosurfaceTrowbridgeReitzSlope(double alphax, double alphay)
    : MicrosurfaceSlope{alphax, alphay} {
}

double MicrosurfaceTrowbridgeReitzSlope::projectedArea(const Vector3d &wi) const {
    if (wi.z() > 0.9999) {
        return 1.0;
    } else if (wi.z() < -0.9999) {
        return 0.0;
    }

    // [Heitz et al. 2016] Appendix B.
    // The projected area can be represented as "(1.0 + Lambda) * cosTheta".
    // However, following representation is more numerically stable.
    const double alpha = alpha_i(wi);
    const double cosTheta = vect::cosTheta(wi);
    const double sinTheta = vect::sinTheta(wi);
    return 0.5 * (cosTheta + std::sqrt(cosTheta * cosTheta + sinTheta * sinTheta * alpha * alpha));
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
    const double h = hDist_->invC1(C1 / std::pow(1.0 - U, 1.0 / lambda));
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

    return eval(wo, wi);
}

Spectrum MicrosurfaceReflection::sample(const Vector3d& wo, Vector3d* wi,
                                        const Point2d& rands, double* pdf,
                                        BxDFType* sampledType) const {
    if (wo.z() == 0.0) return Spectrum(0.0);

    *wi = sampleWi(wo);
    if (!vect::sameHemisphere(wo, *wi)) return Spectrum(0.0);

    *pdf = this->pdf(wo, *wi);
    return f(wo, *wi);
}

double MicrosurfaceReflection::pdf(const Vector3d &wo, const Vector3d &wi) const {
    if (wo.z() <= 0.0 || wi.z() <= 0.0) return 0.0;
    
    const Vector3d wh = vect::normalize(wi + wo);
    const double D = sDist_->D(wh);
    const double G = G1(wo);
    return D * vect::dot(wo, wh) / (4.0 * vect::dot(wo, wh));
}

Spectrum MicrosurfaceReflection::eval(const Vector3d &wo, const Vector3d &wi) const {
    if (wi.z() <= 0.0 || wo.z() <= 0.0) {
        return Spectrum(0.0);
    }

    // Initialization
    Vector3d wr = -wo;
    double hr = 1.0 + hDist_->invC1(0.999);
    Spectrum er(1.0);
    Spectrum sum(0.0);

    // Random walk
    for (int r = 0; r < scatteringOrder_; r++) {
        // Next height
        const double U = random.get1D();
        hr = sampleHeight(wr, hr, U);

        // Leave the surface
        if (hr == INFTY) {
            break;
        }

        // Next event estimation
        const Spectrum phase = evalPhaseFunction(-wr, wi);
        const double shadowing = G1(wi, hr);
        Spectrum I = re_ * er * phase * shadowing;
        if (I.isValid()) {
            sum += I;
        }            
    
        // Update weight and direction
        Vector3d wrPrev = wr;
        wr = samplePhaseFunction(-wr);

        // Update er
        const Vector3d wh = vect::normalize(-wrPrev + wr);
        er *= re_ * fresnel_->evaluate(vect::dot(-wrPrev, wh));

        // Check validity
        if (std::isinf(hr) || std::isnan(hr) || std::isinf(wr.z()) || std::isnan(wr.z())) {
            return Spectrum(0.0);
        }
    }
    
    return sum;
}

Spectrum MicrosurfaceReflection::evalPhaseFunction(const Vector3d &wo, const Vector3d &wi) const {
    const Vector3d wh = vect::normalize(wi + wo);
    if (wh.z() <= 0.0) {
        return Spectrum(0.0);
    }

    // [Heitz et al. 2016] Eq(35)
    const Spectrum F = fresnel_->evaluate(vect::dot(wo, wh));
    return F * sDist_->D_wi(wo, wh) / (4.0 * vect::dot(wo, wh));
}

Vector3d MicrosurfaceReflection::samplePhaseFunction(const Vector3d &wi) const {
    const Point2d U = random.get2D();
    const Vector3d wm = sDist_->sampleD_wi(wi, U);
    return vect::reflect(wi, wm);
}

double MicrosurfaceReflection::misWeight(const Vector3d &wo, const Vector3d &wi) const {
    if (wo.x() == -wi.x() && wo.y() == -wi.y() && wo.z() == -wi.z()) {
        return 1.0;
    }

    const Vector3d wh = vect::normalize(wi + wo);
    return sDist_->D(wh);
}

// ----------------------------------------------------------------------------
// Microsurface Fresnel
// ----------------------------------------------------------------------------

MicrosurfaceFresnel::MicrosurfaceFresnel(const Spectrum &re, const Spectrum &tr, double etaA, double etaB,
                                               MicrosurfaceHeight *hDist, MicrosurfaceSlope *sDist, int scatteringOrder)
    : MicrosurfaceDistribution{BxDFType::Reflection | BxDFType::Transmission | BxDFType::Glossy, hDist, sDist, scatteringOrder}
    , re_{re}
    , tr_{tr}
    , etaA_{etaA}
    , etaB_{etaB}
    , fresnel_{new FresnelDielectric(etaA_, etaB_)} {
}

Spectrum MicrosurfaceFresnel::f(const Vector3d& wo, const Vector3d& wi) const {
    const double cosThetaO = vect::cosTheta(wo);
    const double cosThetaI = vect::cosTheta(wi);
    if (cosThetaI == 0.0 || cosThetaO == 0.0) return Spectrum(0.0);

    // [Heitz et al. 2014] Eq.(43)
    const Spectrum e = eval(wo, wi);
    return e;
}

Spectrum MicrosurfaceFresnel::sample(const Vector3d& wo, Vector3d* wi,
                                        const Point2d& rands, double* pdf,
                                        BxDFType* sampledType) const {
    if (wo.z() == 0.0) return Spectrum(0.0);

    *wi = sampleWi(wo);
    *pdf = this->pdf(wo, *wi);
    if (sampledType) {
        *sampledType = vect::sameHemisphere(wo, *wi) ? BxDFType::Reflection | BxDFType::Glossy 
                                                     : BxDFType::Transmission | BxDFType::Glossy;
    }

    return f(wo, *wi);
}

double MicrosurfaceFresnel::pdf(const Vector3d &wo, const Vector3d &wi) const {
    const bool woOutside = wo.z() > 0.0;
    const double absCosThetaO = std::abs(vect::cosTheta(wo));
    if (vect::sameHemisphere(wo, wi)) {
        // Reflection
        Vector3d wh = vect::normalize(wi + wo);
        const double cosTheta = vect::absDot(wo, wh);
        return woOutside ? sDist_->D( wh) * vect::absDot(wo, wh) / (4.0 * vect::dot( wo,  wh))
                         : sDist_->D(-wh) * vect::absDot(wo, wh) / (4.0 * vect::dot(-wo, -wh));
    } else {
        // Transmission
        double eta = vect::cosTheta(wo) > 0.0 ? (etaB_ / etaA_) : (etaA_ / etaB_);
        Vector3d wh = -vect::normalize(wo + wi * eta);
        const double sign = wh.z() > 0.0 ? 1.0 : -1.0;
        wh *= woOutside ? sign : -sign;

        if (vect::dot(wo, wh) < 0.0) return 0.0;
        
        const double cosTheta = std::abs(vect::dot(wo, wh));
        double sqrtDenom = vect::dot(wo, wh) + eta * vect::dot(wi, wh);
        double dwhdwi = std::abs(eta * eta * vect::dot(wi, wh) / (sqrtDenom * sqrtDenom));
        if (woOutside) {
            return sDist_->D( wh) * vect::absDot(wo, wh) * dwhdwi;
        } else {
            return sDist_->D(-wh) * vect::absDot(wo, wh) * dwhdwi;        
        }
    }
}

Vector3d MicrosurfaceFresnel::sampleWi(const Vector3d &wo) const {
    // Initialization
    Vector3d wr = -wo;
    double hr = 1.0 + hDist_->invC1(0.9999);
    bool outside = wo.z() > 0.0;

    // Random walk
    for (int r = 0; r < scatteringOrder_; r++) {
        // Next height
        const double U = random.get1D();
        hr = outside ? sampleHeight(wr, hr, U) : -sampleHeight(-wr, -hr, U);

        // Leave the microsurface
        if (hr == INFTY || hr == -INFTY) {
            break;
        }

        // Next direction
        wr = samplePhaseFunction(-wr, outside, &outside);
    }

    return wr;
}

Spectrum MicrosurfaceFresnel::eval(const Vector3d &wo, const Vector3d &wi) const {
    // Initialization
    Vector3d wr = -wo;
    double hr = 1.0 + hDist_->invC1(0.999);
    bool woOutside = wo.z() > 0.0;
    const bool wiOutside = wi.z() > 0.0;

    // Random walk
    Spectrum sum(0.0);
    for (int r = 0; r < scatteringOrder_; r++) {
        // Next height
        const double U = random.get1D();
        hr = woOutside ? sampleHeight(wr, hr, U) : -sampleHeight(-wr, -hr, U);

        // Leave the surface
        if (hr == INFTY || hr == -INFTY) {
            break;
        }
  
        // Next event estimation
        const Spectrum phase = evalPhaseFunction(-wr, wi, woOutside, wiOutside);
        const double shadowing = wi.z() > 0.0 ? G1(wi, hr) : G1(-wi, -hr);        
        Spectrum I = phase * shadowing * (vect::sameHemisphere(-wr, wi) ? re_ : tr_);
        if (I.isValid()) {
            sum += I;
        }

        // Next direction
        wr = samplePhaseFunction(-wr, woOutside, &woOutside);
        if (wr.z() == 0.0) {
            return Spectrum(0.0);
        }

        // Check validity
        if (std::isinf(hr) || std::isnan(hr) || std::isinf(wr.z()) || std::isnan(wr.z())) {
            return Spectrum(0.0);
        }
    }    
    return sum;
}

Spectrum MicrosurfaceFresnel::evalPhaseFunction(const Vector3d &wo, const Vector3d &wi) const {
    // This method is not used in Fresnel case.
    Warning("Deprecated method \"MicrosurfaceFresnel::evalPhaseFunction\" is called");
    return Spectrum(0.0);
}

Spectrum MicrosurfaceFresnel::evalPhaseFunction(const Vector3d &wo, const Vector3d &wi, bool woOutside, bool wiOutside) const {
    if (woOutside == wiOutside) {
        // Reflection
        const Vector3d wh = vect::normalize(wi + wo);
        const double cosTheta = vect::absDot(wo, wh);
        const double F = woOutside ? FrDielectric(cosTheta, etaA_, etaB_) : FrDielectric(cosTheta, etaB_, etaA_);
        double ret = woOutside ? F * sDist_->D_wi( wo,  wh) / (4.0 * vect::dot( wo,  wh))
                               : F * sDist_->D_wi(-wo, -wh) / (4.0 * vect::dot(-wo, -wh));
        return Spectrum(ret);
    } else {
        // Transmission
        const double eta = woOutside ? etaB_ / etaA_ : etaA_ / etaB_;
        Vector3d wh = -vect::normalize(wo + wi * eta);
        const double sign = wh.z() >= 0.0 ? 1.0 : -1.0;
        wh *= woOutside ? sign : -sign;

        if (vect::dot(wh, wo) < 0.0) return Spectrum(0.0);

        double ret;
        if (woOutside) {
            const double cosTheta = std::abs(vect::dot(wo, wh));
            const double F = FrDielectric(cosTheta, etaA_, etaB_);
            const double denom = vect::dot(wo, wh) + eta * vect::dot(wi, wh);
            ret = (1.0 - F) * eta * eta * sDist_->D_wi(wo, wh) * std::max(0.0, -vect::dot(wi, wh)) / (denom * denom);
        } else {
            const double cosTheta = std::abs(vect::dot(wo, wh));
            const double F = FrDielectric(cosTheta, etaB_, etaA_);
            const double denom = vect::dot(-wo, -wh) + eta * vect::dot(-wi, -wh);
            ret = (1.0 - F) * eta * eta * sDist_->D_wi(-wo, -wh) * std::max(0.0, -vect::dot(-wi, -wh)) / (denom * denom);        
        }
        return Spectrum(ret);
    }
}

Vector3d MicrosurfaceFresnel::samplePhaseFunction(const Vector3d &wi) const {
    // This method is not used in Fresnel case.
    Warning("Deprecated method \"MicrosurfaceFresnel::samplePhaseFunction\" is called");
    return Vector3d(0.0, 0.0, 1.0);
}

Vector3d MicrosurfaceFresnel::samplePhaseFunction(const Vector3d &wo, bool woOutside, bool *wiOutside) const {
    const Point2d U = random.get2D();
    const Vector3d wm = woOutside ?  sDist_->sampleD_wi( wo, U)
                                  : -sDist_->sampleD_wi(-wo, U);

    const double cosTheta = vect::absDot(wo, wm);
    const double F = woOutside ? FrDielectric(cosTheta, etaA_, etaB_) : FrDielectric(cosTheta, etaB_, etaA_);

    if (random.get1D() < F) {
        // Reflection
        return vect::reflect(wo, wm);
    } else {
        // Transmission
        const double eta = woOutside ? etaA_ / etaB_ : etaB_ / etaA_;
        Vector3d wi;
        if (!vect::refract(wo, wm, eta, &wi)) {
            return Vector3d(0.0, 0.0, 0.0);
        }
        *wiOutside = !woOutside;
        return wi.normalized();
    }
}

double MicrosurfaceFresnel::misWeight(const Vector3d &wo, const Vector3d &wi) const {
    if (vect::sameHemisphere(wo, wi)) {
        // Reflection
        if (wo.x() == -wi.x() && wo.y() == -wi.y() && wo.z() == -wi.z()) {
            return 1.0;
        }

        Vector3d wh = vect::normalize(wo + wi);
        if (wh.z() < 0.0) wh = -wh;
        return sDist_->D(wh);
    } else {
        // Transmission
        const double eta = wo.z() > 0.0 ? etaB_ / etaA_ : etaA_ / etaB_;
        Vector3d wh = vect::normalize(wo + wi * eta);
        if (wh.z() < 0.0) wh = -wh;
        return sDist_->D(wh);
    }
}

}  //namespace spica