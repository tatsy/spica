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
class SPICA_EXPORTS BSSRDF {
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
 * SeparableBSSRDF class.
 */
class SPICA_EXPORTS SeparableBSSRDF : public BSSRDF {
public:
    // Public methods
    SeparableBSSRDF(const SurfaceInteraction& po, double eta,
                    const Material* material);
    Spectrum S(const SurfaceInteraction& pi, const Vector3d& wi) const;
    Spectrum sample(const Scene& scene, double rand1, const Point2d& rand2,
                    MemoryArena& arena, SurfaceInteraction* po,
                    double* pdf) const override;

protected:
    // Protected methods
    Spectrum Sw(const Vector3d& w) const;
    Spectrum Sp(const SurfaceInteraction& pi) const;
    Spectrum sampleSp(const Scene& scene, double rand1, const Point2d& rand2,
                      MemoryArena& arena, SurfaceInteraction* pi,
                      double* pdf) const;
    double pdfSp(const SurfaceInteraction& isect) const;

    // Protected virtual methods
    virtual Spectrum Sr(double r) const = 0;
    virtual double sampleSr(int ch, double rand) const = 0;
    virtual double pdfSr(int ch, double r) const = 0;

private:
    // Private fields
    const Normal3d normal_;
    const Vector3d tangent_, binormal_;
    const Material* material_;

    // Friend
    friend class SeparableBSSRDFAdapter;
};

/**
 * Diffuse BSSRDF class.
 */
class SPICA_EXPORTS DiffuseBSSRDF : public SeparableBSSRDF {
public:
    DiffuseBSSRDF(const SurfaceInteraction& po, const Material* material,
                  double eta, const Spectrum& sigmaAbsorb,
                  const Spectrum& sigmaScatter, const CatmullRom2D& table);

protected:
    Spectrum Sr(double r) const override;
    double sampleSr(int ch, double rand) const override;
    double pdfSr(int ch, double r) const override;

private:
    // Private fields
    const CatmullRom2D& table_;
    Spectrum sigmaExt_, albedo_;
};

/**
 * Adapter for evaluating surface reflection part of BSSRDF.
 */
class SeparableBSSRDFAdapter : public BxDF {
public:
    explicit SeparableBSSRDFAdapter(const SeparableBSSRDF* bssrdf);

    Spectrum f(const Vector3d& wo, const Vector3d& wi) const override;

private:
    const SeparableBSSRDF* bssrdf_;
};

// -----------------------------------------------------------------------------
// BSSRDF utility functions
// -----------------------------------------------------------------------------

SPICA_EXPORTS double FresnelMoment1(double eta);
SPICA_EXPORTS double FresnelMoment2(double eta);

SPICA_EXPORTS
void subsurfaceFromDiffuse(const CatmullRom2D& table, const Spectrum& albedoEff,
                           const Spectrum& mfp, Spectrum* sigA, Spectrum* sigS);

SPICA_EXPORTS 
void computeBeamDiffusionBSSRDF(double g, double eta, CatmullRom2D* table,
                                int albedoDivide = 100, int radiusDivide = 64);

}  // namespace spica

#endif  // _SPICA_BSSRDF_H_
