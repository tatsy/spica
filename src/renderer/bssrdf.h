#ifndef _SPICA_BSSRDF_H_
#define _SPICA_BSSRDF_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_BSSRDF_EXPORT
        #define SPICA_BSSRDF_DLL __declspec(dllexport)
    #else
        #define SPICA_BSSRDF_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_BSSRDF_DLL
#endif

#include <vector>

#include "../utils/vector3d.h"
#include "../utils/color.h"

namespace spica {

    class BSSRDF;

    // ------------------------------------------------------------
    // Interface class for BSSRDF
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL BSSRDFBase {
    protected:
        double _eta;

    protected:
        explicit BSSRDFBase(double eta = 1.3) : _eta(eta) {}
        explicit BSSRDFBase(const BSSRDFBase& base) : _eta(base._eta) {}
        BSSRDFBase& operator=(const BSSRDFBase& base) { _eta = base._eta; return *this; }

    public:
        virtual ~BSSRDFBase() {}
        virtual double Ft(const Vector3D& nornal, const Vector3D& in) const;
        virtual double Fdr() const;
        virtual Color operator()(const Vector3D& v1, const Vector3D& v2) const = 0;
        virtual BSSRDFBase* clone() const = 0;
    };


    // ------------------------------------------------------------
    // BSSRDF with dipole approximation
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL DipoleBSSRDF : public BSSRDFBase {
    private:
        double _A;
        Color  _sigmap_t;
        Color  _sigma_tr;
        Color  _alphap;
        Color  _zpos;
        Color  _zneg;

    private:
        DipoleBSSRDF();
        DipoleBSSRDF(const Color& sigma_a, const Color& sigmap_s, double eta = 1.3);
        DipoleBSSRDF(const DipoleBSSRDF& bssrdf);
        DipoleBSSRDF& operator=(const DipoleBSSRDF& bssrdf);

    public:
        static BSSRDF factory(const Color& sigma_a, const Color& sigmap_s, double eta = 1.3);
        Color operator()(const Vector3D& v1, const Vector3D& v2) const override;
        BSSRDFBase* clone() const override;
    };

    // ------------------------------------------------------------
    // BSSRDF with diffuse reflectance function Rd
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL DiffuseBSSRDF : public BSSRDFBase {
    private:
        std::vector<double> _distances;
        std::vector<Color> _colors;

    public:
        DiffuseBSSRDF();
        DiffuseBSSRDF(const double eta, const std::vector<double>& distances, const std::vector<Color>& colors);
        DiffuseBSSRDF(const DiffuseBSSRDF& bssrdf);
        DiffuseBSSRDF(DiffuseBSSRDF&& bssrdf);
        DiffuseBSSRDF& operator=(const DiffuseBSSRDF& bssrdf);
        DiffuseBSSRDF& operator=(DiffuseBSSRDF&& bssrdf);

        static BSSRDF factory(const double eta, const std::vector<double>& distances, const std::vector<Color>& colors);
        BSSRDF factory() const;
        Color operator()(const Vector3D& v1, const Vector3D& v2) const override;
        BSSRDFBase* clone() const override;

        int numIntervals() const;
        const std::vector<double>& distances() const;
        const std::vector<Color>& colors() const;

        DiffuseBSSRDF scaled(double sc) const;
        void load(const std::string& filename);
        void save(const std::string& filename) const;
    };

    // ------------------------------------------------------------
    // Custom BSSRDF interface
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL CustomBSSRDF : public BSSRDFBase {
    protected:
        CustomBSSRDF* _ptr;

    public:
        CustomBSSRDF();
        virtual ~CustomBSSRDF();
        virtual BSSRDF factory();
        virtual Color operator()(const Vector3D& v1, const Vector3D& v2) const override = 0;
        virtual BSSRDFBase* clone() const override = 0;
    };
 
    // ------------------------------------------------------------
    // Abstract BSSRDF class
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL BSSRDF {
    private:
        const BSSRDFBase* _ptr;

    public:
        BSSRDF();
        BSSRDF(const BSSRDF& bssrdf);
        BSSRDF(BSSRDF&& bssrdf);
        ~BSSRDF();

        BSSRDF& operator=(const BSSRDF& bssrdf);
        BSSRDF& operator=(BSSRDF&& bssrdf);

        double Ft(const Vector3D& normal, const Vector3D& in) const;
        double Fdr() const;
        Color operator()(const Vector3D& v1, const Vector3D& v2) const;

    private:
        explicit BSSRDF(const BSSRDFBase* ptr);
        void nullCheck() const;

        // Friend classes
        friend class DipoleBSSRDF;
        friend class DiffuseBSSRDF;
        friend class CustomBSSRDF;
    };

}

#endif  // _SPICA_BSSRDF_H_
