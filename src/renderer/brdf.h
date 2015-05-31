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
#include "../utils/vector3.h"
#include "../utils/color.h"

namespace spica {

    class SPICA_BRDF_DLL BRDF {
    public:
        virtual ~BRDF() {}
        virtual Color reflectance() const = 0;
        virtual void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const = 0;
    };

    class SPICA_BRDF_DLL LambertianBRDF : public BRDF {
    private:
        Color _reflectance;

    public:
        LambertianBRDF(const Color& reflectance);
        Color reflectance() const;
        void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const;
    };

    class SPICA_BRDF_DLL SpecularBRDF : public BRDF {
    private:
        Color _reflectance;

    public:
        SpecularBRDF(const Color& reflectance);
        Color reflectance() const;
        void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const;
    };

    class SPICA_BRDF_DLL PhongBRDF : public BRDF {
    private:
        Color _reflectance;
        double _coeffN;

    public:
        PhongBRDF(const Color& reflectance, const double n);
        Color reflectance() const;
        void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const;
    };

}

#endif  // _SPICA_BRDF_H_
