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
        Vector3D _p;  //! position
        Vector3D _n;  //! normal
        Color    _e;  //! light emittance

    public:
        LightSample(const Vector3D& p = Vector3D(0.0, 0.0, 0.0),
                  const Vector3D& n = Vector3D(0.0, 0.0, 0.0),
                  const Color& e = Color(0.0, 0.0, 0.0))
            : _p(p)
            , _n(n)
            , _e(e) {
        }

        ~LightSample() {}

        LightSample(const LightSample& l)
            : _p()
            , _n()
            , _e() {
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
    };

    class SPICA_EXPORTS ILight {
    public:
        ILight() {}
        virtual ~ILight() {}

        virtual LightSample sample(double r1, double r2, double r3) const = 0;
        virtual Color directLight(const Vector3D& dir) const = 0;
        virtual double area() const = 0;
        virtual ILight* clone() const = 0;
    };

}  // spica

#endif  // _SPICA_LIGHT_INTERFACE_
