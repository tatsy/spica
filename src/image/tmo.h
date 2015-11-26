#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TMO_H_
#define _SPICA_TMO_H_

#include "../core/common.h"
#include "../core/forward_decl.h"

namespace spica {

    /** Base class for tone mapping operators.
     */
    class Tmo {
    public:
        Tmo() {}
        virtual ~Tmo() {}

        virtual Image apply(const spica::Image& image) const = 0;
    };

    /** Gamma correction.
     */
    class SPICA_EXPORTS GammaTmo : public Tmo {
    private:
        double _gamma;

    public:
        explicit GammaTmo(double gamma);
        ~GammaTmo();
        Image apply(const Image& image) const override;
    };


    /** Reinhard TMO.
     */
    class SPICA_EXPORTS ReinhardTmo : public Tmo {
    private:
        double _alpha;

    public:
        explicit ReinhardTmo(double alpha = 0.18);
        ~ReinhardTmo();
        Image apply(const Image& image) const override;

    private:
        static double whitePoint(const Image& image);
    };

    /** Drago TMO.
     */
    class SPICA_EXPORTS DragoTmo : public Tmo {
    private:
        double _Ldmax;
        double _p;

    public:
        explicit DragoTmo(double Ldmax=100.0, double p=0.85);
        ~DragoTmo();
        Image apply(const Image& image) const override;
    };

}  // namespace spica

#endif  // _SPICA_TMO_H_
