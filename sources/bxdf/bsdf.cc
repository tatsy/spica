#define SPICA_API_EXPORT
#include "bsdf.h"

#include "../core/interaction.h"
#include "../math/vect_math.h"

#include "bxdf.h"

namespace spica {

BSDF::BSDF(const SurfaceInteraction& isect, double eta)
    : eta_{ eta }
    , normal_{ isect.normal().normalized() }
    , tangent_{ isect.dpdu().normalized() }
    , binormal_{ isect.dpdv().normalized() } {
}

void BSDF::add(BxDF* b) {
    Assertion(nBxDFs_ < maxBxDFs_, "Number of BxDF is over size limit!!");
    bxdfs_[nBxDFs_++] = b;
}

int BSDF::numComponents(BxDFType type) const {
    int ret = 0;
    for (int i = 0; i < nBxDFs_; i++) {
        if ((bxdfs_[i]->type() & type) == bxdfs_[i]->type()) {
            ret++;
        }
    }
    return ret;
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
    for (int i = 0; i < nBxDFs_; i++) {
        if (((bxdfs_[i]->type() & type) != BxDFType::None) &&
            ((reflect && ((bxdfs_[i]->type() & BxDFType::Reflection) != BxDFType::None)) ||
             (!reflect && ((bxdfs_[i]->type() & BxDFType::Transmission) != BxDFType::None)))) {
            ret += bxdfs_[i]->f(wo, wi);
        }
    }
    return ret;
}

Spectrum BSDF::sample(const Vector3D& woWorld, Vector3D* wiWorld,
                      const Point2D& rands, double* pdf, BxDFType type,
                      BxDFType* sampledType) const {
    int matchComps = numComponents(type);
    if (matchComps == 0) {
        *pdf = 0.0;
        if (sampledType) *sampledType = BxDFType::None;
        return Spectrum(0.0);
    }

    int comps = std::min((int)(rands[0] * matchComps), matchComps - 1);

    BxDF* bxdf = nullptr;
    int count = comps;
    for (int i = 0; i < nBxDFs_; i++) {
        if ((bxdfs_[i]->type() & type) != BxDFType::None && count-- == 0) {
            bxdf = bxdfs_[i];
            break;
        }
    }
    Assertion(bxdf, "BxDF not found!!");

    Point2D uRemapped(rands[0] * matchComps - comps, rands[1]);

    Vector3D wi, wo = worldToLocal(woWorld);
    *pdf = 0.0;
    if (sampledType) *sampledType = bxdf->type();
    Spectrum ret = bxdf->sample(wo, &wi, uRemapped, pdf, sampledType);
    if (*pdf == 0.0) {
        if (sampledType) *sampledType = BxDFType::None;
        return Spectrum(0.0);
    }
    *wiWorld = localToWorld(wi);

    if ((bxdf->type() & BxDFType::Specular) == BxDFType::None && matchComps > 1) {
        for (int i = 0; i < nBxDFs_; i++) {
            if (bxdfs_[i] != bxdf && (bxdfs_[i]->type() & type) != BxDFType::None) {
                *pdf += bxdfs_[i]->pdf(wo, wi);
            }
        }
    }
    if (matchComps > 1) *pdf /= matchComps;

    if ((bxdf->type() & BxDFType::Specular) == BxDFType::None && matchComps > 1) {
        ret = Spectrum(0.0);
        for (int i = 0; i < nBxDFs_; i++) {
            bool reflect = vect::dot(*wiWorld, normal_) * vect::dot(woWorld, normal_) > 0.0;
            if ((bxdfs_[i]->type() & type) != BxDFType::None &&
                ((reflect && (bxdfs_[i]->type() & BxDFType::Reflection) != BxDFType::None) ||
                 (!reflect && (bxdfs_[i]->type() & BxDFType::Transmission) != BxDFType::None))) {
                ret += bxdfs_[i]->f(wo, wi);
            }
        }
    }
    return ret;
}

double BSDF::pdf(const Vector3D& woWorld, const Vector3D& wiWorld, BxDFType type) const {
    if (nBxDFs_ == 0) return 0.0;
    Vector3D wo = worldToLocal(woWorld), wi = worldToLocal(wiWorld);
    double pdf = 0.0;
    int matchComps = 0;
    for (int i = 0; i < nBxDFs_; i++) {
        if ((bxdfs_[i]->type() & type) != BxDFType::None) {
            pdf += bxdfs_[i]->pdf(wo, wi);
            matchComps++;
        }
    }
    return matchComps > 0 ? pdf / matchComps : 0.0;
}

bool BSDF::hasType(BxDFType type) const {
    for (int i = 0; i < nBxDFs_; i++) {
        if ((bxdfs_[i]->type() & type) != BxDFType::None) return true;
    }
    return false;
}

}  //namespace spica
