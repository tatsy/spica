#define SPICA_API_EXPORT
#include "fresnel.h"

#include <algorithm>

namespace spica {

// -----------------------------------------------------------------------------
// FresnelConductor method definitions
// -----------------------------------------------------------------------------

FresnelConductor::FresnelConductor(const Spectrum& etaI, const Spectrum& etaT,
                                   const Spectrum& k)
    : etaI_{ etaI }
    , etaT_{ etaT }
    , k_{ k } {
}

Spectrum FresnelConductor::evaluate(double cosThetaI) const {
    cosThetaI = std::abs(cosThetaI);
    cosThetaI = clamp(cosThetaI, -1.0, 1.0);
    Spectrum eta = etaT_ / etaI_;
    Spectrum etaK = k_ / etaI_;

    double cosThetaI2 = cosThetaI * cosThetaI;
    double sinThetaI2 = 1.0 - cosThetaI2;
    Spectrum eta2 = eta * eta;
    Spectrum etaK2 = etaK * etaK;

    Spectrum t0 = eta2 - etaK2 - sinThetaI2;
    Spectrum a2plusb2 = Spectrum::sqrt(t0 * t0 + 4.0 * eta2 * etaK2);
    Spectrum t1 = a2plusb2 * cosThetaI2;
    Spectrum a = Spectrum::sqrt(0.5 * (a2plusb2 + t0));
    Spectrum t2 = 2.0 * cosThetaI * a;
    Spectrum Rs = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Spectrum t4 = t2 * sinThetaI2;
    Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5 * (Rp + Rs);
}

// -----------------------------------------------------------------------------
// FresnelDielectric method definitions
// -----------------------------------------------------------------------------

FresnelDielectric::FresnelDielectric(double etaI, double etaT)
    : etaI_{ etaI }
    , etaT_{ etaT } {
}

Spectrum FresnelDielectric::evaluate(double cosThetaI) const {
    double eI = etaI_;
    double eT = etaT_;

    cosThetaI = clamp(cosThetaI, -1.0, 1.0);

    bool entering = cosThetaI > 0.0;
    if (!entering) {
        std::swap(eI, eT);
        cosThetaI = std::abs(cosThetaI);
    }

    double sinThetaI = std::sqrt(std::max(0.0, 1.0 - cosThetaI * cosThetaI));
    double sinThetaT = eI / eT * sinThetaI;

    if (sinThetaT >= 1.0) return Spectrum(1.0);  // Total reflection

    double cosThetaT = std::sqrt(std::max(0.0, 1.0 - sinThetaT * sinThetaT));
    double Rpara = ((eT * cosThetaI) - (eI * cosThetaT)) /
                   ((eT * cosThetaI) + (eI * cosThetaT));
    double Rperp = ((eI * cosThetaI) - (eT * cosThetaT)) /
                   ((eI * cosThetaI) + (eT * cosThetaT));
    return Spectrum((Rpara * Rpara + Rperp * Rperp) / 2.0);
}

}  // namespace spica
