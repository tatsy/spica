#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LIGHT_INTERFACE_
#define _SPICA_LIGHT_INTERFACE_

#include "../core/forward_decl.h"
#include "../math/vector3d.h"
#include "../core/spectrum.h"
#include "../core/stack.h"

namespace spica {

    class SPICA_EXPORTS LightSample {
    private:
        Vector3D _pos{0.0, 0.0, 0.0};  /** Position.      */
        Vector3D _nrm{0.0, 0.0, 0.0};  /** Normal.        */
        Vector3D _dir{0.0, 0.0, 0.0};  /** Out direction. */
        Spectrum _emt{0.0, 0.0, 0.0};  /** Emission.      */
        double   _pdf = 0.0;           /** Sample PDF.    */

    public:
        LightSample() {
        }

        LightSample(const Vector3D& p, const Vector3D& n, const Vector3D& dir, const Spectrum& e, double pdf)
            : _pos{p}
            , _nrm{n}
            , _dir{dir}
            , _emt{e}
            , _pdf{pdf} {
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

        inline Vector3D position() const { return _pos; }
        inline Vector3D normal()   const { return _nrm; }
        inline Vector3D dir()      const { return _dir; }
        inline Spectrum Le()       const { return _emt; }
        inline double   pdf()      const { return _pdf; }
    };

    /** Light type enumerator.
     *  @ingroup light_module
     */
    enum class LightType {
        None      = 0x00,  /**< Light type not specified. */
        Area      = 0x01,  /**< Area light.               */
        Envmap    = 0x02,  /**< Enviroment map.           */
    };

    /**
     * The base class for the lights.
     */
    class SPICA_EXPORTS Light {
    public:
        Light(LightType type) : type_{ type } {}

        Light(const Light& light)
            : type_{ light.type_ } {
        }

        Light(Light&& light)
            : type_{ light.type_ } {
        }

        virtual ~Light() {}

        Light& operator=(const Light& light) {
            type_ = light.type_;
            return *this;
        }

        Light& operator=(Light&& light) {
            type_ = light.type_;
            return *this;
        }

        LightType type() const { return type_; }

        virtual LightSample sample(const Vector3D& v, Stack<double>& rands) const = 0;
        virtual Photon samplePhoton(Stack<double>& rands) const = 0;

        virtual Spectrum directLight(const Vector3D& dir) const = 0;
        virtual Spectrum globalLight(const Vector3D& dir) const = 0;
        virtual double area() const = 0;
        virtual Light* clone() const = 0;

    private:
        LightType type_ = LightType::None;
    };

}  // spica

#endif  // _SPICA_LIGHT_INTERFACE_
