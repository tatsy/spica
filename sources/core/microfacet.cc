#define SPICA_API_EXPORT
#include "microfacet.h"

#include <cmath>

#include "core/point2d.h"
#include "core/vector3d.h"
#include "core/math.h"
#include "core/vect_math.h"

namespace spica {

namespace {

static void beckmannSample11(const double cosThetaI, double U1, double U2, double *slopex, double *slopey) {
    // Special case (normal incidence)
    // This case is equivarent to isotropic roughness case
    if (cosThetaI > 0.9999) {
        const double r = std::sqrt(-std::log(1.0 - U1));
        const double sinPhi = std::sin(2.0 * PI * U2);
        const double cosPhi = std::cos(2.0 * PI * U2);
        *slopex = r * cosPhi;
        *slopey = r * sinPhi;
        return;
    }

    // Solve inverse of C22 with the bisection method
    // The following approach is reported in [Jakob 2014]
    // "An Improved Visible Normal Sampling Routine for the Beckmann Distribution"
    const double sinThetaI = std::sqrt(std::max(0.0, 1.0 - cosThetaI * cosThetaI));
    const double tanThetaI = sinThetaI / cosThetaI;
    const double cotThetaI = 1.0 / tanThetaI;

    double a = -1.0;
    double c = math::erf(cotThetaI);
    double samplex = std::max(U1, 1.0e-6);

    double thetaI = std::acos(cosThetaI);
    double fit = 1.0 + thetaI * (-0.876 + thetaI * (0.4265 - 0.0594 * thetaI));
    double b = c - (1.0 + c) * std::pow(1.0 - samplex, fit);

    // Normalization factor below equals to (G1(w) / 2)^{-1}
    static const double SQRT_PI_INV = 1.0 / std::sqrt(PI);
    const double normalization = 1.0 / (1.0 + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

    for (int it = 0; it < 16; it++) {
        if (b < a || c < b) {
            b = 0.5 * (a + c);
        }

        double xm = math::erfinv(b);
        double value = normalization * (1.0 + b + SQRT_PI_INV * tanThetaI * std::exp(-xm * xm)) - samplex;
        double derivative = normalization * (1.0 - xm * tanThetaI);

        if (std::abs(value) < 1.0e-6) break;

        if (value > 0.0) {
            c = b;
        } else {
            a = b;
        }

        b -= value / derivative;
    }

    *slopex = math::erfinv(b);
    *slopey = math::erfinv(2.0 * std::max(U2, 1.0e-6) - 1.0);
}

static Vector3d beckmannSample(const Vector3d &wi, double alphax, double alphay, double U1, double U2) {
    // Sampling strategy in Algorithm 4 of [Heitz et al. 2014]
    // "Importance Sampling Microfacet-Based BSDFs using the Distribution of Visible Normals"

    // 1. Stretch wi
    Vector3d wiStretched = vect::normalize(Vector3d(alphax * wi.x(), alphay * wi.y(), wi.z()));

    // 2. Simulate P22_{wi}(x_slope, y_slope, 1, 1)
    double slopex, slopey;
    beckmannSample11(vect::cosTheta(wiStretched), U1, U2, &slopex, &slopey);
 
    // 3. Rotate
    const double tmp = vect::cosPhi(wiStretched) * slopex - vect::sinPhi(wiStretched) * slopey;
    slopey = vect::sinPhi(wiStretched) * slopex + vect::cosPhi(wiStretched) * slopey;
    slopex = tmp;

    // 4. Unstretch
    slopex *= alphax;
    slopey *= alphay;

    // 5. Compute normal
    return vect::normalize(Vector3d(-slopex, -slopey, 1.0));
}

}  // anonymous namespace

// -----------------------------------------------------------------------------
// MicrofacetDistribution method definitions
// -----------------------------------------------------------------------------

MicrofacetDistribution::MicrofacetDistribution(double alphax, double alphay, bool sampleVisibleArea)
    : alphax_{alphax}
    , alphay_{alphay}
    , sampleVisibleArea_{sampleVisibleArea} {
}

MicrofacetDistribution::~MicrofacetDistribution() {
}

double MicrofacetDistribution::G1(const Vector3d& w) const {
    return 1.0 / (1.0 + lambda(w));
}

double MicrofacetDistribution::G(const Vector3d& wo, const Vector3d& wi) const {
    return 1.0 / (1.0 + lambda(wo) + lambda(wi));    
}

double MicrofacetDistribution::pdf(const Vector3d& wo, const Vector3d& wh) const {
    if (sampleVisibleArea_) {
        return D(wh) * G1(wo) * vect::absDot(wo, wh) / std::abs(vect::cosTheta(wo));
    } else {
        return D(wh) * std::abs(vect::cosTheta(wh));
    }
}

// -----------------------------------------------------------------------------
// TrowbridgeReitzDistribution method definitions
// -----------------------------------------------------------------------------

TrowbridgeReitzDistribution::TrowbridgeReitzDistribution(double alphax,
                                                         double alphay,
                                                         bool samplevis)
    : MicrofacetDistribution{alphax, alphay, samplevis} {
}

double TrowbridgeReitzDistribution::D(const Vector3d& wh) const {
    double tan2Theta = vect::tan2Theta(wh);
    if (std::isinf(tan2Theta)) return 0.0;

    double cos4Theta = vect::cos2Theta(wh) * vect::cos2Theta(wh);
    double e = (vect::cos2Phi(wh) / (alphax_ * alphax_) +
                vect::sin2Phi(wh) / (alphay_ * alphay_)) * tan2Theta;
    return 1.0 / (PI * alphax_ * alphay_ * cos4Theta * (1.0 + e) * (1.0 + e));
}

Vector3d TrowbridgeReitzDistribution::sample(const Vector3d& wo,
                                             const Point2d& rands) const {
    Vector3d wh;
    if (!sampleVisibleArea_) {
        double cosTheta = 0.0;
        double phi = (2.0 * PI) * rands[1];
        if (alphax_ == alphay_) {
            double tanTheta2 = alphax_ * alphax_ * rands[0] / (1.0 - rands[0]);
            cosTheta = 1.0 / std::sqrt(1.0 + tanTheta2);
        } else {
            phi = std::atan(alphay_ / alphax_ * tan(2.0 * PI * rands[1] + 0.5 * PI));
            if (rands[1] > 0.5) {
                phi += PI;
            }
            double sinPhi = std::sin(phi);
            double cosPhi = std::cos(phi);
            double alphax2 = alphax_ * alphax_;
            double alphay2 = alphay_ * alphay_;
            double alpha2 = 
                1.0 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
            double tanTheta2 = alpha2 * rands[0] / (1.0 - rands[0]);
            cosTheta = 1.0 / std::sqrt(1.0 + tanTheta2);
        }
        double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
        wh = Vector3d(std::cos(phi) * sinTheta,
                      std::sin(phi) * sinTheta,
                      cosTheta);
        if (!vect::sameHemisphere(wo, wh)) wh = -wh;
    } else {
        bool flip = wo.z() < 0.0;
        wh = sampleVA(flip ? -wo : wo, alphax_, alphay_, rands);
        if (flip) wh = -wh;
    }
    return wh;
}

double TrowbridgeReitzDistribution::roughnessToAlpha(double rough) {
    rough = std::max(rough, 1.0e-3);
    double x = std::log(rough);
    return 1.62142 + x * (0.819955 + x * (0.1734 +  x * (0.0171201 + 0.000640711 * x)));
}

double TrowbridgeReitzDistribution::lambda(const Vector3d& w) const {
    double absTanTheta = std::abs(vect::tanTheta(w));
    if (std::isinf(absTanTheta)) return 0.0;

    double alpha = std::sqrt(vect::cos2Phi(w) * alphax_ * alphax_ + 
                             vect::sin2Phi(w) * alphay_ * alphay_);
    double tmp = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1.0 + std::sqrt(1.0 + tmp)) / 2.0;
}

Vector3d TrowbridgeReitzDistribution::sampleVA(const Vector3d& wi,
                                               double alphax, double alphay,
                                               const Point2d& rands) {
    Vector3d wiStretched = 
        Vector3d(alphax * wi.x(), alphay * wi.y(), wi.z()).normalized();
    
    double slopex, slopey;
    sampleSlopes(vect::cosTheta(wiStretched), rands, &slopex, &slopey);

    double cosPhi = vect::cosPhi(wiStretched);
    double sinPhi = vect::sinPhi(wiStretched);
    double tmp = cosPhi * slopex - sinPhi * slopey;

    slopey = sinPhi * slopex + cosPhi * slopey;
    slopex = tmp;

    slopex = slopex * alphax;
    slopey = slopey * alphay;

    return Vector3d(-slopex, -slopey, 1.0).normalized();
}

void TrowbridgeReitzDistribution::sampleSlopes(double cosTheta,
                                               const Point2d& rands,
                                               double* slopex, double* slopey) {
    if (cosTheta > 0.9999) {
        double r = std::sqrt(rands[0] / (1.0 - rands[0]));
        double phi = 2.0 * PI * rands[1];
        *slopex = r * std::cos(phi);
        *slopey = r * std::sin(phi);
        return;
    }

    double sinTheta = sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
    double tanTheta = sinTheta / cosTheta;
    double a = 1.0 / tanTheta;
    double G1 = 2.0 / (1.0 + std::sqrt(1.0 + 1.0 / (a * a)));

    double A = 2.0 * rands[0] / G1 - 1.0;
    double tmp = 1.0 / (A * A - 1.0);
    if (tmp > 1.0e10) tmp = 1.0e10;
    double B = tanTheta;
    double D = sqrt(std::max(0.0, B * B * tmp * tmp - (A * A - B * B) * tmp));
    double slopex1 = B * tmp - D;
    double slopex2 = B * tmp + D;
    *slopex = (A < 0.0 || slopex2 > 1.0 / tanTheta) ? slopex1 : slopex2;
    Assertion(!std::isinf(*slopex), "slopex is infinity.");
    Assertion(!std::isnan(*slopex), "slopex is NaN.");

    double S, U;
    if (rands[1] > 0.5) {
        S = 1.0;
        U = 2.0 * (rands[1] - 0.5);
    } else {
        S = -1.0;
        U = 2.0 * (0.5 - rands[1]);
    }

    double z = (U * (U * (U * 0.27385 - 0.73369) + 0.46341)) / 
               (U * (U * (U * 0.093073 + 0.309420) - 1.0) + 0.597999);
    *slopey = S * z * std::sqrt(1.0 + (*slopex) * (*slopex));
    Assertion(!std::isinf(*slopey), "slopey is infinity.");
    Assertion(!std::isnan(*slopey), "slopey is NaN");
}


// -----------------------------------------------------------------------------
// BeckmannDistribution method definitions
// -----------------------------------------------------------------------------

BeckmannDistribution::BeckmannDistribution(double alphax, double alphay, bool sampleVis)
    : MicrofacetDistribution{alphax, alphay, sampleVis} {
}

double BeckmannDistribution::D(const Vector3d &wh) const {
    // See P.15 of [Heitz et al. 2014]
    // "Understanding the Masking-Shadowing Function in Micorfacet-Based BRDFs"
    const double tan2Theta = vect::tan2Theta(wh);
    if (std::isinf(tan2Theta)) return 0.0;

    const double cos4Theta = vect::cos2Theta(wh) * vect::cos2Theta(wh);
    const double alpha_b_2 = vect::cos2Phi(wh) / (alphax_ * alphax_) +
                             vect::sin2Phi(wh) / (alphay_ * alphay_);

    return std::exp(-tan2Theta * alpha_b_2) / (PI * alphax_ * alphay_ * cos4Theta);
}

Vector3d BeckmannDistribution::sample(const Vector3d &wo, const Point2d &rands) const {
    if (!sampleVisibleArea_) {
        double tan2Theta, phi;
        if (alphax_ == alphay_) {
            // Sample theta_m and phi_m on the isotropic rough surface.
            // See Eq.(28) and (29) of [Walter et al. 2007].
            // "Microfacet Models for Refraction through Rough Surfaces"
            const double logSample = std::log(1 - rands[0]);
            Assertion(!std::isinf(logSample), "Invalid log sample detected!");

            tan2Theta = -alphax_ * alphax_ * logSample;
            phi = 2.0 * PI * rands[1];
        } else {
            // Sample theta_m and phi_m on the anisotropic rough surface.
            const double logSample = std::log(1 - rands[0]);
            Assertion(!std::isinf(logSample), "Invalid log sample detected!");

            // Derived from the equation on P.15 of [Heitz et al. 2014]
            phi = std::atan(alphay_ / alphax_ * std::tan(2.0 * PI * rands[1] + 0.5 * PI));
            if (rands[1] > 0.5) {
                phi += PI;
            }

            const double sinPhi = std::sin(phi);
            const double cosPhi = std::cos(phi);
            const double alphax2 = alphax_ * alphax_;
            const double alphay2 = alphay_ * alphay_;
            tan2Theta = -logSample / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        }

        const double cosTheta = 1.0 / std::sqrt(1.0 + tan2Theta);
        const double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
        const double cosPhi = std::cos(phi);
        const double sinPhi = std::sin(phi);
        Vector3d wh(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
        if (!vect::sameHemisphere(wo, wh)) wh = -wh;
        return wh;
    } else {
        Vector3d wh;
        bool flip = wo.z() < 0.0;
        wh = beckmannSample(flip ? -wo : wo, alphax_, alphay_, rands[0], rands[1]);
        if (flip) wh = -wh;
        return wh;
    }
}

double BeckmannDistribution::roughnessToAlpha(double rough) {
    rough = std::max(rough, 1.0e-3);
    double x = std::log(rough);
    return 1.62142 + x * (0.819955 + x * (0.1734 +  x * (0.0171201 + 0.000640711 * x)));
}

double BeckmannDistribution::lambda(const Vector3d &w) const {
    const double absTanTheta = std::abs(vect::tanTheta(w));
    if (std::isinf(absTanTheta)) {
        return 0.0;
    }

    const double alpha = std::sqrt(vect::cos2Phi(w) * alphax_ * alphax_ + vect::sin2Phi(w) * alphay_ * alphay_);
    const double a = 1.0 / (alpha * absTanTheta);
    if (a >= 1.6) return 0.0;

    // Below is an approximation of following equation in [0, 1.6] 
    // (erf(a) - 1) / 2 + (1 / (2 * a * sqrt(pi)) * exp(-a * a)
    return (1.0 - 1.259 * a + 0.396 * a * a) / (3.535 * a + 2.181 * a * a);
}

}  // namespace spica
