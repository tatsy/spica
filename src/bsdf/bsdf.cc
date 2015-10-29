#define SPICA_API_EXPORT
#include "bsdf.h"

#include "brdf.h"
#include "bssrdf.h"

#include "../renderer/renderer_constants.h"
#include "../renderer/renderer_helper.h"
#include "../renderer/subsurface_integrator.h"

namespace spica {

    BSDF::BSDF()
        : _ptr{}
        , _bssrdf{}
        , _type{BsdfType::None} {
    }

    BSDF::BSDF(const BSDF& bsdf)
        : BSDF{} {
        this->operator=(bsdf);
    }

    BSDF::BSDF(BSDF&& bsdf)
        : BSDF{} {
        this->operator=(std::move(bsdf));
    }

    BSDF::BSDF(const BSDFBase* ptr, BsdfType type)
        : _ptr{ptr}
        , _bssrdf{}
        , _type{type} {
    }

    BSDF::~BSDF() {
    }

    BSDF& BSDF::operator=(const BSDF& bsdf) {
        if (bsdf._ptr) {
            _ptr.reset(bsdf._ptr->clone());
        }

        if (bsdf._bssrdf) {
            _bssrdf.reset(new BSSRDF(*bsdf._bssrdf.get()));
        }

        _type = bsdf._type;
        return *this;
    }

    BSDF& BSDF::operator=(BSDF&& bsdf) {
        if (bsdf._ptr) {
            _ptr.reset(bsdf._ptr.release());
        }

        if (bsdf._bssrdf) {
            _bssrdf.reset(bsdf._bssrdf.release());
        }

        _type = bsdf._type;
        return *this;
    }

    const Color& BSDF::reflectance() const {
        return _ptr->reflectance();
    }

    void BSDF::sample(const Vector3D& in, const Vector3D& normal,
                      double rand1, double rand2,
                      Vector3D* out, double* pdf) const {
        _ptr->sample(in, normal, rand1, rand2, out, pdf);
    }

    Color BSDF::evalBSSRDF(const Vector3D& in, 
                             const Vector3D& pos,
                             const Vector3D& normal,
                             const SubsurfaceIntegrator& integr,
                             double* refPdf) const {
        // Transmitted radiance
        Color transRad(0.0, 0.0, 0.0);

        // Fresnel reflection
        const Vector3D orientN = in.dot(normal) < 0.0 ? normal : -normal;
        const bool into = normal.dot(orientN) > 0.0;
        Vector3D refdir, transdir;
        double fresnelRe, fresnelTr;
        if (helper::checkTotalReflection(into, in, normal, orientN,
                                            &refdir, &transdir,
                                            &fresnelRe, &fresnelTr)) {
            (*refPdf) = 1.0;
        } else {
            (*refPdf) = (1.0 / fresnelRe);
            transRad = integr.irradiance(pos, *this) * fresnelTr;
        }
        return transRad;
    }

    BsdfType BSDF::type() const {
        return _type;
    }

    void BSDF::setBssrdf(const BSSRDF& bssrdf) {
        _bssrdf.reset(new BSSRDF(bssrdf));
        this->_type = this->_type | BsdfType::Bssrdf;
    }

}  // namespace spica
