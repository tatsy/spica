#ifndef _SPICA_BSSRDF_H_
#define _SPICA_BSSRDF_H_

#include <vector>
#include <memory>

#include "../core/forward_decl.h"
#include "../core/spectrum.h"

namespace spica {

    // ------------------------------------------------------------
    // Interface class for BSSRDF
    // ------------------------------------------------------------

    class SPICA_EXPORTS BSSRDFBase {
    public:
        virtual ~BSSRDFBase() {}
        virtual double Ft(const Normal& nornal, const Vector3D& in) const;
        virtual double Fdr() const;
        virtual Spectrum operator()(const Point& v1, const Point& v2) const = 0;
        virtual BSSRDFBase* clone() const = 0;

    protected:
        // Protected methods
        explicit BSSRDFBase(double eta = 1.3) : _eta(eta) {}
        explicit BSSRDFBase(const BSSRDFBase& base) : _eta(base._eta) {}
        BSSRDFBase& operator=(const BSSRDFBase& base) { _eta = base._eta; return *this; }

        // Protected fields
        double _eta;
    };


    // ------------------------------------------------------------
    // BSSRDF with dipole approximation
    // ------------------------------------------------------------

    class SPICA_EXPORTS DipoleBSSRDF : public BSSRDFBase {
    private:
        double _A;
        Spectrum  _sigmap_t;
        Spectrum  _sigma_tr;
        Spectrum  _alphap;
        Spectrum  _zpos;
        Spectrum  _zneg;

    private:
        DipoleBSSRDF();
        DipoleBSSRDF(const Spectrum& sigma_a, const Spectrum& sigmap_s, double eta = 1.3);
        DipoleBSSRDF(const DipoleBSSRDF& bssrdf);
        DipoleBSSRDF& operator=(const DipoleBSSRDF& bssrdf);

    public:
        static BSSRDF factory(const Spectrum& sigma_a, const Spectrum& sigmap_s, double eta = 1.3);
        Spectrum operator()(const Point& v1, const Point& v2) const override;
        BSSRDFBase* clone() const override;
    };

    // ------------------------------------------------------------
    // BSSRDF with diffuse reflectance function Rd
    // ------------------------------------------------------------

    class SPICA_EXPORTS DiffuseBSSRDF : public BSSRDFBase {
    public:
        DiffuseBSSRDF();
        DiffuseBSSRDF(const double eta, const std::vector<double>& distances, const std::vector<Spectrum>& colors);
        DiffuseBSSRDF(const DiffuseBSSRDF& bssrdf);
        DiffuseBSSRDF(DiffuseBSSRDF&& bssrdf);
        DiffuseBSSRDF& operator=(const DiffuseBSSRDF& bssrdf);
        DiffuseBSSRDF& operator=(DiffuseBSSRDF&& bssrdf);

        static BSSRDF factory(const double eta, const std::vector<double>& distances, const std::vector<Spectrum>& colors);
        BSSRDF factory() const;
        Spectrum operator()(const Point& v1, const Point& v2) const override;
        BSSRDFBase* clone() const override;

        int numIntervals() const;
        const std::vector<double>& distances() const;
        const std::vector<Spectrum>& colors() const;

        DiffuseBSSRDF scaled(double sc) const;
        void load(const std::string& filename);
        void save(const std::string& filename) const;

    private:
        std::vector<double>   _distances;
        std::vector<Spectrum> _colors;
    };

    // ------------------------------------------------------------
    // Custom BSSRDF interface
    // ------------------------------------------------------------

    class SPICA_EXPORTS CustomBSSRDF : public BSSRDFBase {
    protected:
        std::unique_ptr<const CustomBSSRDF> _ptr;

    public:
        CustomBSSRDF();
        virtual ~CustomBSSRDF();
        virtual BSSRDF factory();
        virtual Spectrum operator()(const Point& v1, const Point& v2) const override = 0;
        virtual BSSRDFBase* clone() const override = 0;
    };
 
    // ------------------------------------------------------------
    // Abstract BSSRDF class
    // ------------------------------------------------------------

    class SPICA_EXPORTS BSSRDF {
    private:
        std::unique_ptr<const BSSRDFBase> _ptr;

    public:
        BSSRDF();
        BSSRDF(const BSSRDF& bssrdf);
        BSSRDF(BSSRDF&& bssrdf);
        ~BSSRDF();

        BSSRDF& operator=(const BSSRDF& bssrdf);
        BSSRDF& operator=(BSSRDF&& bssrdf);

        double Ft(const Normal& normal, const Vector3D& in) const;
        double Fdr() const;
        Spectrum operator()(const Point& v1, const Point& v2) const;

    private:
        explicit BSSRDF(const BSSRDFBase* ptr);
        void nullCheck() const;

        // Friend classes
        friend class DipoleBSSRDF;
        friend class DiffuseBSSRDF;
        friend class CustomBSSRDF;
    };

}  // namespace spica

#endif  // _SPICA_BSSRDF_H_
