#define SPICA_RENDERER_HELPER_EXPORT
#include "renderer_helper.h"

#include <cmath>

namespace spica {

    namespace helper {
        bool isTotalRef(const bool isIncoming,
                        const Vector3& position,
                        const Vector3& in,
                        const Vector3& normal,
                        const Vector3& orientNormal,
                        Vector3* reflectDir,
                        Vector3* refractDir,
                        double* fresnelRef,
                        double* fresnelTransmit) {

            *reflectDir = Vector3::reflect(in, normal);

            // Snell's rule
            const double nnt = isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM;
            const double ddn = in.dot(orientNormal);
            const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

            if (cos2t < 0.0) {
                // Total reflect
                *refractDir = Vector3();
                *fresnelRef = 1.0;
                *fresnelTransmit = 0.0;
                return true;
            }

            *refractDir = (in * nnt - normal * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalized();

            const double a = IOR_OBJECT - IOR_VACCUM;
            const double b = IOR_OBJECT + IOR_VACCUM;
            const double R0 = (a * a) / (b * b);

            const double c = 1.0 - (isIncoming ? -ddn : Vector3::dot(*refractDir, -orientNormal));
            *fresnelRef = R0 + (1.0 - R0) * pow(c, 5.0);
            *fresnelTransmit = 1.0 - (*fresnelRef);

            return false;
        }

    }

}