#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LIGHT_INTERFACE_
#define _SPICA_LIGHT_INTERFACE_

#include "../math/vector3d.h"
#include "../core/color.h"
#include "../renderer/ray.h"

namespace spica {

    class SPICA_EXPORTS LightSample {
    private:
        Vector3D _p{0.0, 0.0, 0.0};  /** Position.   */
        Vector3D _n{0.0, 0.0, 0.0};  /** Normal.     */
        Color    _e{0.0, 0.0, 0.0};  /** Emission.   */
        double   _area = 0.0;        /** Light area. */

    public:
        LightSample() {
        }

        LightSample(const Vector3D& p, const Vector3D& n, const Color& e, double area)
            : _p{p}
            , _n{n}
            , _e{e}
            , _area{area} {
        }

        ~LightSample() {
        }

        LightSample(const LightSample& l)
            : LightSample{} {
            this->operator=(l);
        }

        LightSample& operator=(const LightSample& l) {
            this->_p = l._p;
            this->_n = l._n;
            this->_e = l._e;
            return *this;
        }

        inline Vector3D position() const { return _p; }
        inline Vector3D normal()   const { return _n; }
        inline Color    Le()       const { return _e; }
        inline double   area()     const { return _area; }
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

        virtual LightSample sample(double r1, double r2, double r3) const = 0;
        virtual Color directLight(const Vector3D& dir) const = 0;
        virtual Color globalLight(const Vector3D& dir) const = 0;
        virtual double area() const = 0;
        virtual ILight* clone() const = 0;
    };

}  // spica

#endif  // _SPICA_LIGHT_INTERFACE_
