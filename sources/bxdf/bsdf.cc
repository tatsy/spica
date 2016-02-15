#define SPICA_API_EXPORT
#include "bsdf.h"

#include "../core/interaction.h"
#include "../math/vect_math.h"

namespace spica {

BSDF::BSDF(const SurfaceInteraction& isect, double eta)
    : eta_{ eta }
    , normal_{ isect.normal() }
    , tangent_{ isect.dpdu() }
    , binormal_{ isect.dpdv() } {
}

void BSDF::add(BxDF* b) {
    Assertion(nBxDFs < maxBxDFs_, "Number of BxDF is over size limit!!");
    bxdfs_[nBxDFs++] = b;
}

Vector3D BSDF::worldToLocal(const Vector3D& v) const {
    double x = vect::dot(tangent_, v);
    double y = vect::dot(binormal_, v);
    double z = vect::dot(normal_, v);
    return { x, y, z };
}

Vector3D BSDF::localToWorld(const Vector3D& v) const {
    return v.x() * tangent_ + v.y() * binormal_ + v.z() * Vector3D(normal_);
}

Spectrum BSDF::f(const Vector3D& wiWorld, const Vector3D& woWorld,
                 BxDFType type) const {
    Vector3D wi = worldToLocal(wiWorld);
    Vector3D wo = worldToLocal(woWorld);
    bool reflect = vect::dot(wiWorld, normal_) * vect::dot(woWorld, normal_) > 0.0;
    Spectrum ret(0.0);
    for (int i = 0; i < nBxDFs; i++) {
        if ((bxdfs_[i]->type() & type) &&
            ((reflect && (bxdfs_[i]->type() & BxDFType::Reflection)) ||
             (!reflect && (bxdfs_[i]->type() & BxDFType::Transmission)))) {
            ret += bxdfs_[i]->f(wo, wi);
        }
    }
    return ret;
}

Spectrum BSDF::sample(const Vector3D& woWorld, Vector3D* wiWorld,
                      const Point2D& rands, double* pdf, BxDFType type) const {
    int matchComps = numComponents(type);
    int comps = std::min((int)(rands[0] * matchComps), matchComps - 1);

    BxDF* bxdf = nullptr;
    int count = comps;
    for (int i = 0; i < nBxDFs; i++) {
        if (bxdfs_[i]->type() & type && count-- == 0) {
            bxdf = bxdfs_[i];
            break;
        }
    }
    Assertion(bxdf, "BxDF not found!!");

    Point2D uRemapped(rands[0] * matchComps - comps, rands[1]);

    Vector3D wi, wo = worldToLocal(wo);
    Spectrum ret = bxdf->sample(wo, &wi, uRemapped, pdf);

    *wiWorld = localToWorld(wi);

    if (!(bxdf->type() & BxDFType::Specular) && matchComps > 1) {
        for (int i = 0; i < nBxDFs; i++) {
            if (bxdfs_[i] != bxdf && (bxdfs_[i]->type() & type)) {
                *pdf += bxdfs_[i]->pdf(wo, wi);
            }
        }
    }
    if (matchComps > 1) *pdf /= matchComps;

    if (!(bxdf->type() & BxDFType::Specular) && matchComps > 1) {
        ret = Spectrum(0.0);
        for (int i = 0; i < nBxDFs; i++) {
            bool reflect = vect::dot(*wiWorld, normal_) * vect::dot(woWorld, normal_) > 0.0;
            if ((bxdfs_[i]->type() & type) &&
                ((reflect && bxdfs_[i]->type() & BxDFType::Reflection) ||
                 (!reflect && bxdfs_[i]->type() & BxDFType::Transmission))) {

            }
        }
    }
    return ret;
}

double BSDF::pdf(const Vector3D& woWorld, const Vector3D& wiWorld, BxDFType type) const {
    if (nBxDFs == 0) return 0.0;
    Vector3D wo = worldToLocal(woWorld), wi = worldToLocal(wiWorld);
    double pdf = 0.0;
    int matchComps = 0;
    for (int i = 0; i < nBxDFs; i++) {
        if (bxdfs_[i]->type() & type) {
            pdf += bxdfs_[i]->pdf(wo, wi);
            matchComps++;
        }
    }
    return matchComps > 0 ? pdf / matchComps : 0.0;
}

bool BSDF::hasType(BxDFType type) const {
    for (int i = 0; i < nBxDFs; i++) {
        if (bxdfs_[i]->type() & type) return true;
    }
    return false;
}

}  //namespace spica
