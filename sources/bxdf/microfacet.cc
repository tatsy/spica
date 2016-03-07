#define SPICA_API_EXPORT
#include "microfacet.h"

#include "../core/point2d.h"
#include "../math/vector3d.h"
#include "../math/vect_math.h"

namespace spica {

// -----------------------------------------------------------------------------
// MicrofacetDistribution method definitions
// -----------------------------------------------------------------------------

MicrofacetDistribution::MicrofacetDistribution(bool sampleVisibleArea)
    : sampleVisibleArea_{ sampleVisibleArea } {
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
    : MicrofacetDistribution{ samplevis }
    , alphax_{ alphax }
    , alphay_{ alphay } {
}

double TrowbridgeReitzDistribution::D(const Vector3d& wh) const {
    double tan2Theta = vect::tan2Theta(wh);
    if (isinf(tan2Theta)) return 0.0;

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
    if (isinf(absTanTheta)) return 0.0;

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
    Assertion(!isinf(*slopex), "slopex is infinity.");
    Assertion(!isnan(*slopex), "slopex is NaN.");

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
    Assertion(!isinf(*slopey), "slopey is infinity.");
    Assertion(!isnan(*slopey), "slopey is NaN");
}

}  // namespace spica
