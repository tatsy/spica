#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BSDF_H_
#define _SPICA_BSDF_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_BSDF_EXPORT
        #define SPICA_BSDF_DLL __declspec(dllexport)
    #else
        #define SPICA_BSDF_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_BSDF_DLL
#endif

#include <memory>

#include "../utils/vector3d.h"
#include "../utils/color.h"

namespace spica {

    // Forward declarations
    class BSDFBase;
    class BSSRDF;
    class SubsurfaceIntegrator;

    // Types of BSDF
    enum class BsdfType : int {
        Bssrdf      = 0x0100,
        Scatter     = 0x0200,
        Dielectric  = 0x0400,

        None        = 0x0000,
        Lambertian  = 0x0001 | Scatter,
        Specular    = 0x0002 | Dielectric,
        PhongBrdf   = 0x0004 | Scatter,
        Reflactive  = 0x0008 | Dielectric,
    };

    inline BsdfType operator|(BsdfType t1, BsdfType t2) {
        return static_cast<BsdfType>((static_cast<int>(t1) | static_cast<int>(t2)));
    }

    inline bool operator&(BsdfType t1, BsdfType t2) {
        return (static_cast<int>(t1) & static_cast<int>(t2)) != 0;
    }

    // --------------------------------------------------
    // BSDF (Bidirectional Scattering Distributioh Function)
    // --------------------------------------------------
    class SPICA_BSDF_DLL BSDF {
    private:
        std::unique_ptr<const BSDFBase> _ptr;
        std::unique_ptr<const BSSRDF>   _bssrdf;
        BsdfType _type;

    public:
        BSDF();
        BSDF(const BSDF& bsdf);
        BSDF(BSDF&& bsdf);
        ~BSDF();

        BSDF& operator=(const BSDF& bsdf);
        BSDF& operator=(BSDF&& bsdf);

        const Color& reflectance() const;

        void sample(const Vector3D& in, const Vector3D& normal, 
                    double rand1, double rand2,
                    Vector3D* out, double* pdf) const;

        Color sampleBssrdf(const Vector3D& in, 
                           const Vector3D& pos,
                           const Vector3D& normal,
                           double rand1, double rand2,
                           const SubsurfaceIntegrator& integr,
                           Vector3D* out, double* pdf) const;

        BsdfType type() const;
        void setBssrdf(const BSSRDF& bssrdf);

    private:
        BSDF(const BSDFBase* ptr, BsdfType type);

        // Friend classes
        friend class LambertianBRDF;
        friend class SpecularBRDF;
        friend class PhongBRDF;
        friend class RefractiveBSDF;
        friend class SubsurfaceIntegrator;
    };

}  // namespace spica

#endif  // _SPICA_BSDF_H_
