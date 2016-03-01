#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LIGHT_INTERFACE_
#define _SPICA_LIGHT_INTERFACE_

#include "../core/forward_decl.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../core/spectrum.h"
#include "../math/vector3d.h"
#include "../math/transform.h"
#include "../core/uncopyable.h"
#include "../core/stack.h"

namespace spica {

    class SPICA_EXPORTS LightSample {
    private:
        Point3d  _pos{0.0, 0.0, 0.0};  /** Position.      */
        Normal3d _nrm{0.0, 0.0, 0.0};  /** Normal.        */
        Vector3d _dir{0.0, 0.0, 0.0};  /** Out direction. */
        Spectrum _emt{0.0, 0.0, 0.0};  /** Emission.      */
        double   _pdf = 0.0;           /** Sample PDF.    */

    public:
        LightSample() {
        }

        LightSample(const Point3d& p, const Normal3d& n, const Vector3d& dir,
                    const Spectrum& e, double pdf)
            : _pos{ p }
            , _nrm{ n }
            , _dir{ dir }
            , _emt{ e }
            , _pdf{ pdf } {
        }

        ~LightSample() {
        }

        LightSample(const LightSample& l)
            : LightSample{} {
            this->operator=(l);
        }

        LightSample& operator=(const LightSample& l) {
            this->_pos = l._pos;
            this->_nrm = l._nrm;
            this->_dir = l._dir;
            this->_emt = l._emt;
            this->_pdf = l._pdf;
            return *this;
        }

        inline Point3d  position() const { return _pos; }
        inline Normal3d normal()   const { return _nrm; }
        inline Vector3d dir()      const { return _dir; }
        inline Spectrum Le()       const { return _emt; }
        inline double   pdf()      const { return _pdf; }
    };

    /** 
     * Light type enumerator.
     * @ingroup light_module
     */
    enum class LightType {
        None      = 0x00,  /**< Light type not specified. */
        Area      = 0x01,  /**< Area light.               */
        Envmap    = 0x02,  /**< Enviroment map.           */
    };

    /**
     * The base class for the lights.
     */
    class SPICA_EXPORTS Light : public Uncopyable {
    public:
        Light(LightType type, const Transform& light2World, int numSamples = 1);
        virtual ~Light();

        LightType type() const;

        /**
         * Sample incident radiance (Li) at the intersecting point.
         * @param[in] pObj: The intersecting point.
         * @param[in] rands: A pair of real random numbers.
         * @return Sampled incident randiance.
         */
        virtual Spectrum sampleLi(const Interaction& pObj, const Point2d& rands,
                                  Vector3d* dir, double* pdf, VisibilityTester* vis) const = 0;

        /**
         * Compute PDF for the incident direction.
         * @param[in] isect: The intersecting point.
         * @param[in] inDir: Incident direction.
         * @return PDF (probability density).
         */
        virtual double pdfLi(const Interaction& pObj, const Vector3d& dir) const = 0;

        virtual Spectrum Le(const Ray& ray) const;
        virtual Spectrum sampleLe(const Point2d& rand1, const Point2d& rand2,
                                  Ray* ray, Normal3d* nLight, double* pdfPos,
                                  double *pdfDir) const = 0;
        virtual void pdfLe(const Ray& ray, const Normal3d& nLight,
                           double* pdfPos, double* pdfDir) const = 0;

        virtual Spectrum power() const = 0;

        virtual Light* clone() const = 0;

        inline virtual bool isDelta() const { return false; }

    protected:
        const LightType type_;
        const int numSamples_;

        const Transform lightToWorld_;
        const Transform worldToLight_;
    };

}  // spica

#endif  // _SPICA_LIGHT_INTERFACE_
