#ifndef SPICA_MATERIAL_H_
#define SPICA_MATERIAL_H_

#include "../utils/color.h"
#include "../utils/common.h"

namespace spica {

    enum ReflectionType {
        REFLECTION_DIFFUSE    = 0x01,
        REFLECTION_SPECULAR   = 0x02,
        REFLECTION_REFRACTION = 0x04,
        REFLECTION_SUBSURFACE = 0x08,
    };

    const double IOR_VACCUM = 1.0;
    const double IOR_OBJECT = 1.5;
    const double REFLECT_PROBABLITY = 0.5;

    struct Material {
        Color emission;
        Color color;
        ReflectionType reftype;
        Material()
            : emission()
            , color()
            , reftype(REFLECTION_DIFFUSE)
        {
        }

        Material(const Color& emission_, const Color& color_, const ReflectionType reftype_)
            : emission(emission_)
            , color(color_)
            , reftype(reftype_)
        {
        }
    };

    struct DiffusionReflectance {
        double A;
        double sigmap_t;
        double sigma_tr;
        double alphap;
        double zpos;
        double zneg;

        DiffusionReflectance(double sigma_a, double sigmap_s, double eta) {
            A = (1.0 + Fdr(eta)) / (1.0 - Fdr(eta));
            sigmap_t = sigma_a + sigmap_s;
            sigma_tr = sqrt(3.0 * sigma_a * sigmap_t);
            alphap = sigmap_s / sigmap_t;
            zpos = 1.0 / sigmap_t;
            zneg = zpos * (1.0 + (4.0 / 3.0) * A);
        }

        double Fdr(double eta) const {
            if (eta >= 1.0) {
                return -1.4399 / (eta * eta) + 0.7099 / eta + 0.6681 + 0.0636 * eta;
            } else {
                return -0.4399 + 0.7099 / eta - 0.3319 / (eta * eta) + 0.0636 / (eta * eta * eta);
            }
        }

        double Ft(const Vector3& n, const Vector3& in) const {
            const double nnt = IOR_OBJECT / IOR_VACCUM;
            const double ddn = in.dot(n);
            const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

            if (cos2t < 0.0) return 0.0;

            Vector3 refractDir = (in * nnt + n * (ddn * nnt + sqrt(cos2t))).normalized();

            const double a = IOR_OBJECT - IOR_VACCUM;
            const double b = IOR_OBJECT + IOR_VACCUM;
            const double R0 = (a * a) / (b * b);

            const double c  = 1.0 - Vector3::dot(refractDir, -n);
            const double Re = R0 + (1.0 - R0) * pow(c, 5.0);
            return 1.0 - Re;
        }

        double operator()(const double d2) const {
            double dpos = sqrt(d2 + zpos * zpos);
            double dneg = sqrt(d2 + zneg * zneg);
            double posTerm = zpos * (dpos * sigma_tr + 1.0) * exp(-sigma_tr * dpos) / (dpos * dpos * dpos);
            double negTerm = zneg * (dneg * sigma_tr + 1.0) * exp(-sigma_tr * dneg) / (dneg * dneg * dneg);
            double ret = (alphap / (4.0 * PI * sigmap_t)) * (posTerm + negTerm);
            return ret;
        }
    };
}

#endif
