#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BRDF_H_
#define _SPICA_BRDF_H_

#include "../core/common.h"
#include "../math/vector3d.h"
#include "../core/normal3d.h"
#include "../core/spectrum.h"

namespace spica {

    class BSDF;

    /** Abstract class for BSDF object 
     *  @ingroup bsdf_module
     */
    class SPICA_EXPORTS AbstractBSDF {
    public:
        // Public methods
        AbstractBSDF() {}
        virtual ~AbstractBSDF() {}
        virtual const Spectrum& reflectance() const = 0;
        virtual void sample(const Vector3D& in, const Normal& normal, double rand1, double rand2, Vector3D* out, double* pdf) const = 0;
        virtual double pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const = 0;
        virtual AbstractBSDF* clone() const = 0;
    };


    /** Lambertian BRDF.
     *  @ingroup bsdf_module
     */
    class SPICA_EXPORTS LambertianBRDF : public AbstractBSDF {

    public:
        // Public methods
        static BSDF factory(const Spectrum& reflectance);
        const Spectrum& reflectance() const override;
        void sample(const Vector3D& in, const Normal& normal, double rand1, double rand2, Vector3D* out, double* pdf) const override;
        double pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const override;
        AbstractBSDF* clone() const override;

    private:
        // Private methods
        explicit LambertianBRDF(const Spectrum& reflectance);

        // Private fields
        Spectrum reflectance_;
    };


    /** Specular BRDF.
     *  @ingroup bsdf_module
     */
    class SPICA_EXPORTS SpecularBRDF : public AbstractBSDF {
    public:
        // Public methods
        static BSDF factory(const Spectrum& reflectance);
        const Spectrum& reflectance() const override;
        void sample(const Vector3D& in, const Normal& normal, double rand1, double rand2, Vector3D* out, double* pdf) const override;
        double pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const override;
        AbstractBSDF* clone() const override;

    private:
        // Private methods
        explicit SpecularBRDF(const Spectrum& reflectance);

        // Private fields
        Spectrum reflectance_;
    };


    /** Phong BRDF.
     *  @ingroup bsdf_module
     */
    class SPICA_EXPORTS PhongBRDF : public AbstractBSDF {
    public:
        // Public methods
        static BSDF factory(const Spectrum& reflectance, const double n = 32);
        const Spectrum& reflectance() const override;
        void sample(const Vector3D& in, const Normal& normal, double rand1, double rand2, Vector3D* out, double* pdf) const override;
        double pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const override;
        AbstractBSDF* clone() const override;

    private:
        // Private methods
        PhongBRDF(const Spectrum& reflectance, const double n);

        // Private fields
        Spectrum reflectance_;
        double   coeff_;
    };

    /** Refractive BSDF
     *  @ingroup bsdf_module
     */
    class SPICA_EXPORTS RefractiveBSDF : public AbstractBSDF {

    public:
        // Public methods
        static BSDF factory(const Spectrum& reflectance);
        const Spectrum& reflectance() const override;
        void sample(const Vector3D& in, const Normal& normal, double rand1, double rand2, Vector3D* out, double* pdf) const override;
        double pdf(const Vector3D& in, const Normal& normal, const Vector3D& out) const override;
        AbstractBSDF* clone() const override;

    private:
        // Private methods
        RefractiveBSDF(const Spectrum& reflectance);

        // Private fields
        Spectrum reflectance_;
    };

}  // namespace spica

#endif  // _SPICA_BRDF_H_
