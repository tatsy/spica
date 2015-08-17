#define SPICA_BSDF_EXPORT
#include "bsdf.h"

#include "brdf.h"
#include "bssrdf.h"

namespace spica {

    BSDF::BSDF()
        : _ptr()
        , _bssrdf()
        , _type(BSDF_TYPE_NONE)
    {
    }

    BSDF::BSDF(const BSDF& bsdf)
        : _ptr()
        , _bssrdf()
        , _type(BSDF_TYPE_NONE)
    {
        this->operator=(bsdf);
    }

    BSDF::BSDF(BSDF&& bsdf)
        : _ptr()
        , _bssrdf()
        , _type(BSDF_TYPE_NONE)
    {
        this->operator=(std::move(bsdf));
    }

    BSDF::BSDF(const BSDFBase* ptr, BsdfType type)
        : _ptr(ptr)
        , _bssrdf()
        , _type(type)
    {
    }

    BSDF::~BSDF()
    {
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

    const Color& BSDF::emittance() const {
        return _ptr->emittance();
    }

    void BSDF::sample(const Vector3D& in, const Vector3D& normal, double rand1, double rand2, Vector3D* out, double* pdf) const {
    
    }

    BsdfType BSDF::type() const {
        return _type;
    }

    void BSDF::setBssrdf(const BSSRDF& bssrdf) {
        _bssrdf.reset(new BSSRDF(bssrdf));
        this->_type = this->_type | BSDF_TYPE_BSSRDF;
    }

}  // namespace spica
