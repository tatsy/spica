#define SPICA_API_EXPORT
#include "bssrdf.h"

#include <list>

#include "core/memory.h"
#include "core/interpolation.h"

#include "core/scene.h"
#include "core/bsdf.h"
#include "core/fresnel.h"
#include "core/phase.h"
#include "core/material.h"

namespace spica {

// -----------------------------------------------------------------------------
// BSSRDF utility functions
// -----------------------------------------------------------------------------

namespace {

double beamDiffusionMultipleScatter(double sigma_s, double sigma_a, double g,
                                    double eta, double r) {
    static const int nSamples = 100;
    double Ed = 0.0;

    const double sigmap_s = sigma_s * (1.0 - g);
    const double sigmap_t = sigma_a + sigmap_s;
    const double albedop  = sigmap_s / sigmap_t;

    const double D_g = (2.0 * sigma_a + sigmap_s) / (3.0 * sigmap_t * sigmap_t);
    const double sigma_tr = std::sqrt(sigma_a / D_g);

    const double fm1 = FresnelMoment1(eta);
    const double fm2 = FresnelMoment2(eta);
    const double ze = -2.0 * D_g * (1.0 + 3.0 * fm2) / (1.0 - 2.0 * fm1);

    const double cPhi = 0.25 * (1.0 - 2.0 * fm1);
    const double cE   = 0.5 * (1.0 - 3.0 * fm2);
    for (int i = 0; i < nSamples; i++) {
        const double zr = -std::log(1.0 - (i + 0.5) / nSamples) / sigmap_t;

        const double zv = -zr + 2.0 * ze;
        const double dr = std::sqrt(r * r + zr * zr);
        const double dv = std::sqrt(r * r + zv * zv);

        const double phiD = (1.0 / (4.0 * PI * D_g)) *
                            (std::exp(-sigma_tr * dr) / dr -
                             std::exp(-sigma_tr * dv) / dv);

        const double EDn = (1.0 / (4.0 * PI)) *
            (zr * (1.0 + sigma_tr * dr) *
                std::exp(-sigma_tr * dr) / (dr * dr * dr) -
             zv * (1.0 + sigma_tr * dv) *
                std::exp(-sigma_tr * dv) / (dv * dv * dv));

        const double E = phiD * cPhi + EDn * cE;
        const double kappa = 1.0 - std::exp(-2.0 * sigmap_t * (dr + zr));
        Ed += kappa * albedop * albedop * E;
    }
    return Ed / nSamples;
}

double beamDiffusionSingleScatter(double sigma_s, double sigma_a, double g,
                                  double eta, double r) {
    static const int nSamples = 100;

    const double sigma_t = sigma_a + sigma_s;
    const double albedo  = sigma_s / sigma_t;
    const double tCrit   = r * std::sqrt(eta * eta - 1.0);

    double Ess = 0.0;
    for (int i = 0; i < nSamples; i++) {
        double ti = tCrit - std::log(1.0 - (i + 0.5) / nSamples) / sigma_t;
        double d = std::sqrt(r * r + ti * ti);
        double cosThetaO = ti / d;

        Ess += albedo * std::exp(-sigma_t * (d + tCrit))  / (d * d) *
               phase::hg(cosThetaO, g) *
               (1.0 - FrDielectric(-cosThetaO, 1.0, eta)) *
               std::abs(cosThetaO);
    }
    return Ess / nSamples;
}

}  // anonymous namespace

void computeBeamDiffusionBSSRDF(double g, double eta, CatmullRom2D* table,
                                int albedoDivide, int radiusDivide) {
    std::vector<double> radiusSample(radiusDivide);
    radiusSample[0] = 0.0;
    radiusSample[1] = 2.5e-3;
    for (int i = 2; i < radiusDivide; i++) {
        radiusSample[i] = radiusSample[i - 1] * 1.2;
    }

    std::vector<double> albedoSample(albedoDivide);
    for (int i = 0; i < albedoDivide; i++) {
        albedoSample[i] =
            (1.0 - std::exp(-(8.0 * i) / (albedoDivide - 1))) /
            (1.0 - std::exp(-8.0));
    }

    std::vector<std::vector<double>> profile(
        albedoDivide, std::vector<double>(radiusDivide, 0.0));

    for (int i = 0; i < albedoDivide; i++) {
        for (int j = 0; j < radiusDivide; j++) {
            const double albedo = albedoSample[i];
            const double radius = radiusSample[j];
            profile[i][j] = 2.0 * PI * radius *
                (beamDiffusionSingleScatter(albedo, 1.0 - albedo, g, eta, radius) +
                 beamDiffusionMultipleScatter(albedo, 1.0 - albedo, g, eta, radius));
        }
    }

    *table = CatmullRom2D(profile, albedoSample, radiusSample);
}

void subsurfaceFromDiffuse(const CatmullRom2D& table, const Spectrum& albedoEff,
                           const Spectrum& mfp, Spectrum* sigA, Spectrum* sigS) {
    CatmullRom cr(table.xs(), table.marginalY());
    for (int c = 0; c < Spectrum::channels; c++) {
        double albedo = cr.sample(albedoEff[c]);
        (*sigA).ref(c) = albedo / mfp[c];
        (*sigS).ref(c) = (1.0 - albedo) / mfp[c];
    }
}

// -----------------------------------------------------------------------------
// BSSRDF method definitions
// -----------------------------------------------------------------------------

BSSRDF::BSSRDF(const SurfaceInteraction& po, double eta)
    : po_{ po }
    , eta_{ eta } {
}

// -----------------------------------------------------------------------------
// SeparableBSSRDF method definitions
// -----------------------------------------------------------------------------
SeparableBSSRDF::SeparableBSSRDF(const SurfaceInteraction& po,
                                 double eta, const SubsurfaceMaterial* material)
    : BSSRDF{ po, eta }
    , normal_{ po.normal().normalized() }
    , tangent_{ po.dpdu().normalized() }
    , binormal_{ po.dpdv().normalized() }
    , material_{ material } {
}

Spectrum SeparableBSSRDF::S(const SurfaceInteraction& pi,
                            const Vector3d& wi) const {
    const double Ft = FrDielectric(vect::cosTheta(po_.wo()), 1.0, eta_);
    return (1.0 - Ft) * Sp(pi) * Sw(wi);
}

Spectrum SeparableBSSRDF::sample(const Scene& scene, double rand1,
                               const Point2d& rand2, MemoryArena& arena,
                               SurfaceInteraction* pi, double* pdf) const {
    Spectrum sp = sampleSp(scene, rand1, rand2, arena, pi, pdf);
    if (!sp.isBlack()) {
        pi->setBSDF(arena.allocate<BSDF>(*pi));
        pi->bsdf()->add(arena.allocate<SeparableBSSRDFAdapter>(this));
        pi->wo_ = Vector3d(pi->normal());
    }
    return sp;
}

Spectrum SeparableBSSRDF::sampleSp(const Scene& scene, double rand1,
                                 const Point2d& rand2, MemoryArena& arena,
                                 SurfaceInteraction* pi, double* pdf) const {
    // Choose coordinate system for sampling
    Vector3d xAxis, yAxis, zAxis;
    if (rand1 < 0.5) {
        xAxis = tangent_;
        yAxis = binormal_;
        zAxis = Vector3d(normal_);
        rand1 *= 2.0;
    } else if (rand1 < 0.75) {
        xAxis = binormal_;
        yAxis = Vector3d(normal_);
        zAxis = tangent_;
        rand1 = (rand1 - 0.5) * 4.0;
    } else {
        xAxis = Vector3d(normal_);
        yAxis = tangent_;
        zAxis = binormal_;
        rand1 = (rand1 - 0.75) * 4.0;
    }

    // Choose spectral chennel for sampling
    const int ch =
        std::min((int)(rand1 * Spectrum::channels), Spectrum::channels - 1);
    rand1 = rand1 * Spectrum::channels - ch;

    // Sample distance on XY-plane
    const double r = sampleSr(ch, rand2[0]);
    if (r < 0.0) return Spectrum(0.0);
    const double phi = 2.0 * PI * rand2[1];
    // Compute z-coordinate
    const double rMax = sampleSr(ch, 0.9999);
    if (r >= rMax) return Spectrum(0.0);
    const double zCoord = std::sqrt(rMax * rMax - r * r);

    // Compute sampling ray
    Point3d pFrom =
        po_.pos() + (xAxis * std::cos(phi) + yAxis * std::sin(phi)) * r -
        zCoord * zAxis;
    Point3d pTo   = pFrom + 2.0 * zCoord * zAxis;
    Vector3d dir = pTo - pFrom;
    Ray ray(pFrom, dir, std::max(0.0, dir.norm() - EPS));

    // Compute intersection candidates
    std::list<SurfaceInteraction> candidates;
    for (;;) {
        SurfaceInteraction isect;
        if (!scene.intersect(ray, &isect)) break;

        if (isect.primitive()->material()->subsurface().get() == material_) {
            candidates.push_back(isect);
        }
        ray = isect.spawnRayTo(pTo);
        Assertion(candidates.size() < 100, "Candidate too many!!");
    }

    if (candidates.empty()) return Spectrum(0.0);

    // Randomly select interaction from candidates
    const int nFound = static_cast<int>(candidates.size());
    int selectID = clamp((int)(rand1 * nFound), 0, nFound - 1);

    std::list<SurfaceInteraction>::const_iterator it;
    for (it = candidates.cbegin(); it != candidates.cend(); ++it) {
        if (selectID == 0) {
            *pi = *it;
            break;
        }
        selectID -= 1;
    }

    // Compute PDF and spatial BSSRDF term
    *pdf = pdfSp(*pi) / nFound;
    return Sp(*pi);
}

double SeparableBSSRDF::pdfSp(const SurfaceInteraction& pi) const {
    Vector3d d = po_.pos() - pi.pos();
    Vector3d dLocal(vect::dot(tangent_, d), vect::dot(binormal_, d),
                    vect::dot(normal_, d));
    Normal3d nLocal(vect::dot(tangent_, pi.normal()),
                    vect::dot(binormal_, pi.normal()),
                    vect::dot(normal_, pi.normal()));

    double rProj[3] = { std::sqrt(dLocal.y() * dLocal.y() + dLocal.z() * dLocal.z()),
                        std::sqrt(dLocal.z() * dLocal.z() + dLocal.x() * dLocal.x()),
                        std::sqrt(dLocal.x() * dLocal.x() + dLocal.y() * dLocal.y()) };
    double axisProb[3] = { 0.25, 0.25, 0.5 };

    double pdf = 0.0;
    const double chProb = 1.0 / Spectrum::channels;
    for (int axis = 0; axis < 3; axis++) {
        for (int ch = 0; ch < Spectrum::channels; ch++) {
            pdf += pdfSr(ch, rProj[axis]) * std::abs(nLocal[axis]) * chProb * axisProb[axis];
        }
    }
    return pdf;
}

Spectrum SeparableBSSRDF::Sp(const SurfaceInteraction& pi) const {
    return Sr((po_.pos() - pi.pos()).norm());
}

Spectrum SeparableBSSRDF::Sw(const Vector3d& wi) const {
    double c = 1.0 - 2.0 * FresnelMoment1(1.0 / eta_);
    double Ft = FrDielectric(vect::cosTheta(wi), 1.0, eta_);
    return Spectrum((1.0 - Ft) / (c * PI));
}

// -----------------------------------------------------------------------------
// DiffuseBSSRDF method definitions
// -----------------------------------------------------------------------------

DiffuseBSSRDF::DiffuseBSSRDF(const SurfaceInteraction& po,
                             const SubsurfaceMaterial* material,
                             double eta, const Spectrum& sigmaAbsorb,
                             const Spectrum& sigmaScatter,
                             const CatmullRom2D& table)
    : SeparableBSSRDF{ po, eta, material }
    , table_{ table }
    , sigmaAbsorb_{ sigmaAbsorb }
    , sigmaScatter_{ sigmaScatter }
    , eta_{ eta }
    , sigmaExt_{ sigmaAbsorb + sigmaScatter }
    , albedo_{ sigmaScatter / (sigmaExt_ + EPS) } {
}

Spectrum DiffuseBSSRDF::Sr(double r) const {
    Spectrum ret(0.0);
    for (int ch = 0; ch < Spectrum::channels; ch++) {
        const double rOptical = r * sigmaExt_[ch];

        int albedoOffset, radiusOffset;
        double albW[4], radW[4];
        if (!catmullRomWeight(table_.xs(), albedo_[ch], &albW[0], &albW[1],
                              &albW[2], &albW[3], &albedoOffset) ||
            !catmullRomWeight(table_.ys(), rOptical, &radW[0], &radW[1],
                              &radW[2], &radW[3], &radiusOffset)) {
            continue;
        }

        double sr = 0.0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                const double weight = albW[i] * radW[j];
                if (weight != 0.0) {
                    sr += table_(albedoOffset + i, radiusOffset + j) *
                          weight;
                }
            }
        }

        if (rOptical != 0.0) {
            sr /= 2.0 * PI * rOptical;
        }
        ret.ref(ch) = sr;
    }
    ret *= sigmaExt_ * sigmaExt_;

    return Spectrum::clamp(ret);
}

std::unique_ptr<DiffusionReflectance> DiffuseBSSRDF::Rd() const {
    return std::make_unique<DipoleDiffusionReflectance>(sigmaAbsorb_,
                                                        sigmaScatter_, eta_);
}

double DiffuseBSSRDF::sampleSr(int ch, double rand) const {
    if (sigmaExt_[ch] == 0.0) return -1.0;
    return table_.sample(albedo_[ch], rand) / sigmaExt_[ch];
}

double DiffuseBSSRDF::pdfSr(int ch, double r) const {
    const double rOptical = r * sigmaExt_[ch];

    int albedoOffset, radiusOffset;
    double albW[4], radW[4];
    if (!catmullRomWeight(table_.xs(), albedo_[ch], &albW[0], &albW[1],
                          &albW[2], &albW[3], &albedoOffset) ||
        !catmullRomWeight(table_.ys(), rOptical, &radW[0], &radW[1],
                          &radW[2], &radW[3], &radiusOffset)) {
        return 0.0;
    }

    double sr = 0.0, albedoEff = 0.0;
    for (int i = 0; i < 4; i++) {
        if (albW[i] == 0.0) continue;
        albedoEff += table_.marginalY()[albedoOffset + i] * albW[i];
        for (int j = 0; j < 4; j++) {
            if (radW[j] == 0.0) continue;
            sr += table_(albedoOffset + i, radiusOffset + j) *
                    albW[i] * radW[j];
        }
    }

    if (rOptical != 0.0) {
        sr /= 2.0 * PI * rOptical;
    }
    return std::max(0.0, sr * sigmaExt_[ch] * sigmaExt_[ch] / albedoEff);
}


// -----------------------------------------------------------------------------
// DiffusionReflectance method definitions
// -----------------------------------------------------------------------------

DiffusionReflectance::DiffusionReflectance(double eta)
    : eta_{ eta } {
}

double DiffusionReflectance::Ft(const Vector3d& w) const {
    return FrDielectric(vect::cosTheta(w), 1.0, eta_);
}

double DiffusionReflectance::Fdr() const {
    return FrDiffuseReflectance(eta_);
}


// -----------------------------------------------------------------------------
// DipoleDiffusionReflectance method definitions
// -----------------------------------------------------------------------------

DipoleDiffusionReflectance::DipoleDiffusionReflectance(const Spectrum &sigma_a,
                                                       const Spectrum &sigmap_s,
                                                       float eta)
    : DiffusionReflectance{ eta } {
    A_ = (1.0 + Fdr()) / (1.0 - Fdr());
    sigmap_t_ = sigma_a + sigmap_s;
    sigma_tr_ = Spectrum::sqrt(3.0 * sigma_a * sigmap_t_);
    alphap_   = sigmap_s / sigmap_t_;
    zpos_     = Spectrum(1.0) / sigmap_t_;
    zneg_     = -zpos_ * (1.0 + (4.0 / 3.0) * A_);
}

Spectrum DipoleDiffusionReflectance::operator()(const Point3d& po,
                                                const Point3d& pi) const {
    const double d2 = (po - pi).squaredNorm();
    Spectrum dpos = Spectrum::sqrt(d2 + zpos_ * zpos_);
    Spectrum dneg = Spectrum::sqrt(d2 + zneg_ * zneg_);
    Spectrum Rd = (alphap_ / (4.0 * PI)) *
        ((zpos_ * (dpos * sigma_tr_ + 1.0) *
            Spectrum::exp(-sigma_tr_ * dpos)) / (dpos * dpos * dpos) -
            (zneg_ * (dneg * sigma_tr_ + 1.0) *
            Spectrum::exp(-sigma_tr_ * dneg)) / (dneg * dneg * dneg));
    return Spectrum::clamp(Rd);
}


// -----------------------------------------------------------------------------
// BSSRDF utility functions
// -----------------------------------------------------------------------------

double FresnelMoment1(double eta) {
    const double eta2 = eta * eta;
    const double eta3 = eta * eta2;
    const double eta4 = eta * eta3;
    const double eta5 = eta * eta4;
    if (eta < 1.0) {
        return 0.45966 - 1.73965 * eta + 3.37668 * eta2 - 3.904945 * eta3 +
               2.49277 * eta4 - 0.68441 * eta5;
    } else {
        return -4.61686 + 11.1136 * eta - 10.4646 * eta2 + 5.11455 * eta3 -
                1.27198 * eta4 + 0.12746 * eta5;
    }
}

double FresnelMoment2(double eta) {
    const double eta2 = eta * eta;
    const double eta3 = eta * eta2;
    const double eta4 = eta * eta3;
    const double eta5 = eta * eta4;
    if (eta < 1) {
        return 0.27614 - 0.87350 * eta + 1.12077 * eta2 - 0.65095 * eta3 +
               0.07883 * eta4 + 0.04860 * eta5;
    } else {
        const double r_eta = 1 / eta;
        const double r_eta2 = r_eta * r_eta;
        const double r_eta3 = r_eta * r_eta2;
        return -547.033 + 45.3087 * r_eta3 - 218.725 * r_eta2 +
               458.843 * r_eta + 404.557 * eta - 189.519 * eta2 +
               54.9327 * eta3 - 9.00603 * eta4 + 0.63942 * eta5;
    }
}

SeparableBSSRDFAdapter::SeparableBSSRDFAdapter(const SeparableBSSRDF* bssrdf)
    : BxDF{ BxDFType::Reflection | BxDFType::Diffuse }
    , bssrdf_{ bssrdf } {
}

Spectrum SeparableBSSRDFAdapter::f(const Vector3d& wo, const Vector3d& wi) const {
    Spectrum f = bssrdf_->Sw(wi);
    // TODO: Should the transport mode be considered??
    return f;
}

}  // namespace spica
