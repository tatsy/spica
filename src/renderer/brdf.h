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

    class BRDF;

    // Interface class for BRDF object
    class SPICA_BRDF_DLL BRDFBase {
    protected:
        BRDFBase() {}
        explicit BRDFBase(const BRDFBase&) {}

    public:
        virtual ~BRDFBase() {}
        virtual Color reflectance() const = 0;
        virtual void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const = 0;
    };

    class SPICA_BRDF_DLL LambertianBRDF : public BRDFBase {
    private:
        Color _reflectance;

    public:
        static BRDF factory(const Color& reflectance);
        Color reflectance() const;
        void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const;

    private:
        explicit LambertianBRDF(const Color& reflectance);
    };

    class SPICA_BRDF_DLL SpecularBRDF : public BRDFBase {
    private:
        Color _reflectance;

    public:
        static BRDF factory(const Color& reflectance);
        Color reflectance() const;
        void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const;

    private:
        explicit SpecularBRDF(const Color& reflectance);
    };

    class SPICA_BRDF_DLL PhongBRDF : public BRDFBase {
    private:
        Color _reflectance;
        double _coeffN;

    public:
        static BRDF factory(const Color& reflectance, const double n);
        Color reflectance() const;
        void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const;

    private:
        PhongBRDF(const Color& reflectance, const double n);
    };

    class SPICA_BRDF_DLL BRDF {
    private:
        int* _numCopies;
        const BRDFBase* _ptr;

    public:
        BRDF();
        BRDF(const BRDF& brdf);
        ~BRDF();

        BRDF& operator=(const BRDF& brdf);
        Color reflectance() const;
        void sample(const Vector3& in, const Vector3& normal, const double rand1, const double rand2, Vector3* out) const;

    private:
        explicit BRDF(const BRDFBase* ptr);
        void release();

    // Friend classes
        friend class LambertianBRDF;
        friend class SpecularBRDF;
        friend class PhongBRDF;
    };


}

#endif  // _SPICA_BRDF_H_
