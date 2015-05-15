#define SPICA_SAMPLER_EXPORT
#include "sampler.h"

#include <typeinfo>

namespace spica {

    namespace {
        
        Random rng = Random();

    }  // anonymous namespace

    namespace sampler {

        void onHemisphere(const Vector3& normal, Vector3* direction) {
            Vector3 u, v, w;
            w = normal;
            if (abs(w.x()) > EPS) {
                u = Vector3(0.0, 1.0, 0.0).cross(w).normalized();
            } else {
                u = Vector3(1.0, 0.0, 0.0).cross(w).normalized();
            }

            v = w.cross(u);

            const double r1 = 2.0 * PI * rng.nextReal();
            const double r2 = rng.nextReal();
            const double r2s = sqrt(r2);
            *direction = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)).normalized();
        }

        void onSphere(const Sphere& sphere, Vector3* position, Vector3* normal) {
            double s = 2.0 * rng.nextReal() - 1.0;
            double c = sqrt(1.0 - s * s);
            double p = 2.0 * PI * rng.nextReal();
            double x = c * cos(p);
            double y = c * sin(p);
            double z = s;

            *normal = Vector3(x, y, z);
            *position = sphere.radius() * Vector3(x, y, z) + sphere.center();
        }

        void onDisk(const Disk& disk, Vector3* position, Vector3* normal) {
            double r0 = sqrt(rng.nextReal());
            double r1 = rng.nextReal() * (2.0 * PI);
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
            double r1 = rng.nextReal();
            double r2 = rng.nextReal();
            
            *position = quad.p0() + r1 * e1 + r2 * e2;
            *normal = quad.normal();
        }

        void on(const Primitive* primitive, Vector3* position, Vector3* normal) {
            std::string typname = typeid(*primitive).name();
            if (typname == "class spica::Sphere") {
                const Sphere* sphere = reinterpret_cast<const Sphere*>(primitive);
                onSphere(*sphere, position, normal);
            } else if (typname == "class spica::Quad") {
                const Quad* quad = reinterpret_cast<const Quad*>(primitive);
                onQuad(*quad, position, normal);
            } else if (typname == "class spica::Disk") {
                const Disk* disk = reinterpret_cast<const Disk*>(primitive);
                onDisk(*disk, position, normal);
            } else {
                msg_assert(false, ("Invalid geometry type: " + typname).c_str());
            }
        }

    }  // namespace sampler

}  // namespace spica
