#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BSSRDF_H_
#define _SPICA_BSSRDF_H_

#include <vector>
#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/spectrum.h"
#include "../core/interaction.h"

#include "bxdf.h"

namespace spica {

/**
 * BSSRDF Interface.
 */
class BSSRDF {
public:
    BSSRDF(const SurfaceInteraction& po, double eta);

    virtual Spectrum S(const SurfaceInteraction& pi,
                       const Vector3d& wi) const = 0;
    virtual Spectrum sample(const Scene& scene, double rand1, const Point2d& rand2,
                            MemoryArena& arena, SurfaceInteraction* po,
                            double* pdf) const = 0;

protected:
    const SurfaceInteraction po_;
    double eta_;
};

/**
 * Diffuse BSSRDF class.
 */
class DiffuseBSSRDF : public BSSRDF {
public:
    DiffuseBSSRDF(const SurfaceInteraction& po, const Material* material,
                  double eta, const Spectrum& sigmaAbsorb,
                  const Spectrum& sigmaScatter, const CatmullRom2D& table);

    Spectrum S(const SurfaceInteraction& pi, const Vector3d& wi) const override;
    Spectrum sample(const Scene& scene, double rand1, const Point2d& rand2,
                    MemoryArena& arena, SurfaceInteraction* pi,
                    double* pdf) const override;

protected:
    Spectrum sampleSp(const Scene& scene, double rand1, const Point2d& rand2,
                      MemoryArena& arena, SurfaceInteraction* pi,
                      double* pdf) const;
    double pdfSp(const SurfaceInteraction& pi) const;

    Spectrum Sp(const SurfaceInteraction& pi) const;
    Spectrum Sw(const Vector3d& wi) const;

    Spectrum Sr(double r) const;
    double sampleSr(int ch, double rand) const;
    double pdfSr(int ch, double r) const;

private:
    // Private fields
    const Normal3d normal_;
    const Vector3d tangent_, binormal_;
    const Material* material_;
    const CatmullRom2D& table_;
    Spectrum sigmaExt_, albedo_;

    // Friend
    friend class DiffuseBSSRDFAdapter;
};

/**
 * Adapter for evaluating surface reflection part of BSSRDF.
 */
class DiffuseBSSRDFAdapter : public BxDF {
public:
    explicit DiffuseBSSRDFAdapter(const DiffuseBSSRDF* bssrdf);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;

private:
    const DiffuseBSSRDF* bssrdf_;
};

// -----------------------------------------------------------------------------
// BSSRDF utility functions
// -----------------------------------------------------------------------------

SPICA_EXPORTS
void subsurfaceFromDiffuse(const CatmullRom2D& table, const Spectrum& albedoEff,
                           const Spectrum& mfp, Spectrum* sigA, Spectrum* sigS);

SPICA_EXPORTS 
void computeBeamDiffusionBSSRDF(double g, double eta, CatmullRom2D* table,
                                int albedoDivide = 100, int radiusDivide = 64);

}  // namespace spica

#endif  // _SPICA_BSSRDF_H_
