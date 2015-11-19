#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LIGHT_INTERFACE_
#define _SPICA_LIGHT_INTERFACE_

#include "../core/forward_decl.h"
#include "../math/vector3d.h"
#include "../core/color.h"
#include "../core/stack.h"

namespace spica {

    class SPICA_EXPORTS LightSample {
    private:
        Vector3D _pos{0.0, 0.0, 0.0};  /** Position.      */
        Vector3D _nrm{0.0, 0.0, 0.0};  /** Normal.        */
        Vector3D _dir{0.0, 0.0, 0.0};  /** Out direction. */
        Color    _emt{0.0, 0.0, 0.0};  /** Emission.      */
        double   _pdf = 0.0;           /** Sample PDF.    */

    public:
        LightSample() {
        }

        LightSample(const Vector3D& p, const Vector3D& n, const Vector3D& dir, const Color& e, double pdf)
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
        inline Color    Le()       const { return _emt; }
        inline double   pdf()      const { return _pdf; }
    };

    /** Light type enumerator.
     *  @ingroup light_module
     */
    enum class LightType {
        None      = 0x00,  /**< Light type not specified. */
        AreaLight = 0x01,  /**< Area light.               */
        Envmap    = 0x02,  /**< Enviroment map.           */
    };

    class SPICA_EXPORTS ILight {
    private:
        LightType _type = LightType::None;

    public:
        ILight(LightType type) : _type{type} {}

        ILight(const ILight& light)
            : _type{light._type} {
        }

        ILight(ILight&& light)
            : _type{light._type} {
        }

        virtual ~ILight() {}

        ILight& operator=(const ILight& light) {
            _type = light._type;
            return *this;
        }

        ILight& operator=(ILight&& light) {
            _type = light._type;
            return *this;
        }

        LightType type() const { return _type; }

        virtual LightSample sample(const Vector3D& v, Stack<double>& rands) const = 0;
        virtual Photon samplePhoton(Stack<double>& rands) const = 0;

        virtual Color directLight(const Vector3D& dir) const = 0;
        virtual Color globalLight(const Vector3D& dir) const = 0;
        virtual double area() const = 0;
        virtual ILight* clone() const = 0;
    };

}  // spica

#endif  // _SPICA_LIGHT_INTERFACE_
