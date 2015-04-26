#define SPICA_SAMPLER_EXPORT
#include "sampler.h"

namespace spica {

    Random Sampler::rng = Random::getRNG();

    Vector3 Sampler::onDisk(const Vector3& center, const Vector3& normal, double radius) {
        double r0 = sqrt(rng.randReal());
        double r1 = rng.randNorm() * (2.0 * PI);
        double rx = radius * r0 * cos(r1);
        double ry = radius * r0 * sin(r1);
        Vector3 u, v, w;
        w = normal;
        if (abs(w.x()) > EPS) {
            u = Vector3::cross(Vector3(0.0, 1.0, 0.0), w).normalized();
        } else {
            u = Vector3::cross(Vector3(1.0, 0.0, 0.0), w).normalized();
        }
        v = Vector3::cross(u, w);

        return center + u * rx + v * ry;
    }

}
