#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BRDF_H_
#define _SPICA_BRDF_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_BRDF_EXPORT
        #define SPICA_BRDF_DLL __declspec(dllexport)
    #else
        #define SPICA_BRDF_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_BRDF_DLL
#endif

#include "../utils/common.h"
#include "../utils/vector3d.h"
#include "../utils/color.h"

namespace spica {

    class BSDF;

    // Interface class for BSDF object
    class SPICA_BRDF_DLL BSDFBase {
    protected:
        BSDFBase() {}
        explicit BSDFBase(const BSDFBase&) {}

    public:
        virtual ~BSDFBase() {}
        virtual Color reflectance() const = 0;
        virtual Color emittance() const = 0;
        virtual void sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const = 0;
        virtual BSDFBase* clone() const = 0;
    };

    class SPICA_BRDF_DLL LambertianBRDF : public BSDFBase {
    private:
        Color _reflectance;
        Color _emittance;

    public:
        static BSDF factory(const Color& reflectance, const Color& emittance = Color(0.0, 0.0, 0.0));
        Color reflectance() const override;
        Color emittance() const override;
        void sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const override;
        BSDFBase* clone() const override;

    private:
        explicit LambertianBRDF(const Color& reflectance, const Color& emittance);
    };

    class SPICA_BRDF_DLL SpecularBRDF : public BSDFBase {
    private:
        Color _reflectance;
        Color _emittance;

    public:
        static BSDF factory(const Color& reflectance, const Color& emittance = Color(0.0, 0.0, 0.0));
        Color reflectance() const override;
        Color emittance() const override;
        void sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const;
        BSDFBase* clone() const override;

    private:
        explicit SpecularBRDF(const Color& reflectance, const Color& emittance);
    };

    class SPICA_BRDF_DLL PhongBRDF : public BSDFBase {
    private:
        Color _reflectance;
        Color _emittance;
        double _coeff;

    public:
        static BSDF factory(const Color& reflectance, const Color& emittance = Color(0.0, 0.0, 0.0), const double n = 32);
        Color reflectance() const override;
        Color emittance() const override;
        void sample(const Vector3D& in, const Vector3D& normal, const double rand1, const double rand2, Vector3D* out) const;
        BSDFBase* clone() const override;

    private:
        PhongBRDF(const Color& reflectance, const Color& emittance, const double n);
    };

}  // namespace spica

#endif  // _SPICA_BRDF_H_
