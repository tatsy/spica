#define SPICA_SAMPLER_EXPORT
#include "sampler.h"

#include <typeinfo>

namespace spica {

    namespace {
        
        Random rng = Random::getRNG();

    }  // anonymous namespace

    namespace sampler {

        void onDisk(const Disk& disk, Vector3* position, Vector3* normal) {
            double r0 = sqrt(rng.randReal());
            double r1 = rng.randNorm() * (2.0 * PI);
            double rx = disk.radius() * r0 * cos(r1);
            double ry = disk.radius() * r0 * sin(r1);
            Vector3 u, v, w;
            w = disk.normal();
            if (abs(w.x()) > EPS) {
                u = Vector3::cross(Vector3(0.0, 1.0, 0.0), w).normalized();
            } else {
                u = Vector3::cross(Vector3(1.0, 0.0, 0.0), w).normalized();
            }
            v = Vector3::cross(u, w);

            *position = disk.center() + u * rx + v * ry;
            *normal = disk.normal();
        }

        void onQuad(const Quad& quad, Vector3* position, Vector3* normal) {
            Vector3 e1 = quad.p1() - quad.p0();
            Vector3 e2 = quad.p3() - quad.p0();
            double r1 = rng.randReal();
            double r2 = rng.randReal();
            
            *position = r1 * e1 + r2 * e2;
            *normal = quad.normal();
        }

        void on(const Primitive* primitive, Vector3* position, Vector3* normal) {
            std::cout << typeid(primitive).name() << std::endl;
            if (typeid(primitive).name() == "spica::Sphere") {
            }
        }

    }  // namespace sampler

}  // namespace spica
