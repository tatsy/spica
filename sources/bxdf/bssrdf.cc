#define SPICA_API_EXPORT
#include "bssrdf.h"

#include <list>

#include "../core/memory.h"
#include "../core/interpolation.h"
#include "../scenes/scene.h"

#include "bsdf.h"
#include "fresnel.h"

namespace spica {

// -----------------------------------------------------------------------------
// BSSRDF utility functions
// -----------------------------------------------------------------------------

namespace {

double FresnelMoment1(double eta) {
    const double eta2 = eta * eta;
    const double eta3 = eta * eta2;
    const double eta4 = eta * eta3;
    const double eta5 = eta * eta4;
    if (eta < 1.0) {
        return 0.45966f - 1.73965f * eta + 3.37668f * eta2 - 3.904945 * eta3 +
               2.49277f * eta4 - 0.68441f * eta5;
    } else {
        return -4.61686f + 11.1136f * eta - 10.4646f * eta2 + 5.11455f * eta3 -
                1.27198f * eta4 + 0.12746f * eta5;
    }
}

}  // anonymous namespace

// -----------------------------------------------------------------------------
// BSSRDF method definitions
// -----------------------------------------------------------------------------

BSSRDF::BSSRDF(const SurfaceInteraction& po, double eta)
    : po_{ po }
    , eta_{ eta } {
}


// -----------------------------------------------------------------------------
// DiffuseBSSRDF method definitions
// -----------------------------------------------------------------------------

DiffuseBSSRDF::DiffuseBSSRDF(const SurfaceInteraction& po,
                             const Material* material,
                             double eta, const Spectrum& sigmaAbsorb,
                             const Spectrum& sigmaScatter,
                             const CatmullRom2D& table) 
    : BSSRDF{ po, eta }
    , normal_{ po.normal() }
    , tangent_{ po.dpdu() }
    , binormal_{ po.dpdv() }
    , material_{ material }
    , table_{ table }
    , sigmaExt_{ sigmaAbsorb + sigmaScatter }
    , albedo_{ sigmaScatter / (sigmaExt_ + EPS) } {
}

Spectrum DiffuseBSSRDF::S(const SurfaceInteraction& pi,
                          const Vector3D& wi) const {
    double Ft = FrDielectric(vect::cosTheta(po_.wo()), 1.0, eta_);
    return (1.0 - Ft) * Sp(pi) * Sw(wi);
}

Spectrum DiffuseBSSRDF::sample(const Scene& scene, double rand1,
                               const Point2D& rand2, MemoryArena& arena,
                               SurfaceInteraction* pi, double* pdf) const {
    Spectrum sp = sampleSp(scene, rand1, rand2, arena, pi, pdf);
    if (!sp.isBlack()) {
        pi->setBSDF(arena.allocate<BSDF>(pi));
        pi->bsdf()->add((BxDF*)(arena.allocate<DiffuseBSSRDFAdapter>(this)));
        pi->wo_ = Vector3D(pi->normal());
    }
    return sp;
}

Spectrum DiffuseBSSRDF::sampleSp(const Scene& scene, double rand1,
                                 const Point2D& rand2, MemoryArena& arena,
                                 SurfaceInteraction* pi, double* pdf) const {
    // Choose coordinate system for sampling
    Vector3D xAxis, yAxis, zAxis;
    if (rand1 < 0.5) {
        xAxis = tangent_;
        yAxis = binormal_;
        zAxis = Vector3D(normal_);
        rand1 *= 2.0;
    } else if (rand1 < 0.75) {
        xAxis = Vector3D(normal_);
        yAxis = tangent_;
        zAxis = binormal_;
        rand1 = (rand1 - 0.5) * 4.0;
    } else {
        xAxis = binormal_;
        yAxis = Vector3D(normal_);
        zAxis = tangent_;
        rand1 = (rand1 - 0.75) * 4.0;
    }

    // Choose spectral chennel for sampling
    const int ch =
        std::min((int)(rand1 * Spectrum::channels), Spectrum::channels - 1);
    
    // Sample distance on XY-plane
    const double r = sampleSr(ch, rand2[0]);
    if (r < 0.0) return Spectrum(0.0);
    const double phi = 2.0 * PI * rand2[1];

    // Compute z-coordinate
    const double rMax = sampleSr(ch, 0.9999);
    if (r > rMax) return Spectrum(0.0);
    const double zCoord = std::sqrt(rMax * rMax - r * r);

    // Compute sampling ray
    Point3D pFrom = Point(xAxis * std::cos(phi) + yAxis * std::sin(phi)) * r -
                    zCoord * zAxis;
    Point3D pTo   = pFrom + 2.0 * zCoord * zAxis;
    Ray ray(pFrom, (pTo - pFrom).normalized(), (pTo - pFrom).norm());

    // Compute intersection candidates
    std::list<SurfaceInteraction> candidates;
    SurfaceInteraction isect;
    while (scene.intersect(ray, &isect)) {
        if (isect.primitive()->material() == material_) {
            candidates.push_back(isect);
        }
        ray = isect.spawnRayTo(pTo);
    }

    if (candidates.empty()) return Spectrum(0.0);

    // Randomly select interaction from candidates
    const int nFound   = static_cast<int>(candidates.size());
    int selectID = std::min((int)(rand1 * nFound), nFound);

    std::list<SurfaceInteraction>::const_iterator it;
    for (it = candidates.cbegin(); it != candidates.cend(); ++it) {
        selectID -= 1;
        if (selectID == 0) {
            *pi = *it;
            break;
        }
    }

    // Compute PDF and spatial BSSRDF term
    *pdf = pdfSp(*pi) / nFound;
    return Sp(*pi);
}

double DiffuseBSSRDF::pdfSp(const SurfaceInteraction& pi) const {
    Vector3D d = po_.pos() - pi.pos();
    Vector3D dLocal(vect::dot(tangent_, d), vect::dot(binormal_, d),
                    vect::dot(normal_, d));
    Normal3D nLocal(vect::dot(tangent_, pi.normal()),
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

Spectrum DiffuseBSSRDF::Sp(const SurfaceInteraction& pi) const {
    return Sr((po_.pos() - pi.pos()).norm());
}

Spectrum DiffuseBSSRDF::Sw(const Vector3D& wi) const {
    double c = 1.0 - 2.0 * FresnelMoment1(1.0 / eta_);
    double Ft = FrDielectric(vect::cosTheta(po_.wo()), 1.0, eta_);
    return Spectrum((1.0 - Ft) / (c * PI));
}

Spectrum DiffuseBSSRDF::Sr(double r) const {
}

double DiffuseBSSRDF::sampleSr(int ch, double rand) const {
    if (sigmaExt_[ch] == 0.0) return -1.0;
    return table_.sample(albedo_[ch], rand) / sigmaExt_[ch];
}

double DiffuseBSSRDF::pdfSr(int ch, double r) const {
    const double rOptical = r * sigmaExt_[ch];

    int albedoOffset, radiusOffset;
    double albW[4], radW[4];
    if (!table_.catmullRomX().weights(albedo_[ch], &albW[0], &albW[1],
                                      &albW[2], &albW[3], &albedoOffset) ||
        !table_.catmullRomY().weights(rOptical, &radW[0], &radW[1],
                                      &radW[2], &radW[3], &radiusOffset)) {
        return 0.0;
    }

    double sr = 0.0, albedoEff = 0.0;
    for (int i = 0; i < 4; i++) {
        albedoEff += table_.merginalY()[albedoOffset + i] + albW[i];
        for (int j = 0; j < 4; j++) {
            if (radW[j] != 0.0) {
                sr += table_.evaluate(albedoOffset + i, radiusOffset + j) *
                      albW[i] * radW[j];
            }
        }
    }

    if (rOptical != 0.0) {
        sr /= 2.0 * PI * rOptical;
    }
    return std::max(0.0, sr * sigmaExt_[ch] * sigmaExt_[ch] / albedoEff);
}

}  // namespace spica
