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

#include "../utils/vector3.h"
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
        BSSRDFBase(double eta = 1.3) : _eta(eta) {}
        BSSRDFBase(const BSSRDFBase&) {}

    public:
        virtual ~BSSRDFBase() {}
        virtual double Ft(const Vector3& nornal, const Vector3& in) const;
        virtual double Fdr() const;
        virtual Color operator()(const double d2) const = 0;
    };


    // ------------------------------------------------------------
    // BSSRDF with diffusion approximation
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL DiffusionBSSRDF : public BSSRDFBase {
    private:
        double _A;
        double _sigmap_t;
        double _sigma_tr;
        double _alphap;
        double _zpos;
        double _zneg;

    private:
        DiffusionBSSRDF();
        DiffusionBSSRDF(double sigma_a, double sigmap_s, double eta = 1.3);

    public:
        static BSSRDF factory(double sigma_a, double sigmap_s, double eta = 1.3);
        Color operator()(const double d2) const;
    };

    // ------------------------------------------------------------
    // BSSRDF with discrete Rd
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL DiscreteBSSRDF : public BSSRDFBase {
    private:
        std::vector<double> _distances;
        std::vector<Color> _colors;

    private:
        DiscreteBSSRDF();
        DiscreteBSSRDF(const double eta, const std::vector<double>& distances, const std::vector<Color>& colors);

    public:
        static BSSRDF factory(const double eta, const std::vector<double>& distances, const std::vector<Color>& colors);
        Color operator()(const double d2) const;
    };


    // ------------------------------------------------------------
    // Abstract BSSRDF class
    // ------------------------------------------------------------

    class SPICA_BSSRDF_DLL BSSRDF {
    private:
        int* _numCopies;
        const BSSRDFBase* _ptr;

    public:
        BSSRDF();
        BSSRDF(const BSSRDF& bssrdf);
        ~BSSRDF();

        BSSRDF& operator=(const BSSRDF& bssrdf);

        double Ft(const Vector3& normal, const Vector3& in) const;
        double Fdr() const;
        Color operator()(const double d2) const;

    private:
        BSSRDF(const BSSRDFBase* ptr);
        void release();
        void nullCheck() const;

    // Friend classes
        friend class DiffusionBSSRDF;
        friend class DiscreteBSSRDF;
    };

}

#endif  // _SPICA_BSSRDF_H_
