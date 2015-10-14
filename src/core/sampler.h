#ifndef SPICA_SAMPLER_H_
#define SPICA_SAMPLER_H_

#include "common.h"
#include "../math/vector3d.h"
#include "../shape/shape.h"
#include "../random/random.h"

namespace spica {

    namespace sampler {

        SPICA_EXPORTS void onHemisphere(const Vector3D& normal, Vector3D* direction);

        SPICA_EXPORTS void onHemisphere(const Vector3D& normal, Vector3D* direction, double r1, double r2);

        SPICA_EXPORTS void onSphere(const Sphere& sphere, Vector3D* position, Vector3D* normal);

        SPICA_EXPORTS void onSphere(const Sphere& sphere, Vector3D* position, Vector3D* normal, double r1, double r2);
    
        SPICA_EXPORTS void onDisk(const Disk& disk, Vector3D* position, Vector3D* normal);

        SPICA_EXPORTS void onTriangle(const Triangle& tri, Vector3D* position, Vector3D* normal);

        SPICA_EXPORTS void onTriangle(const Triangle& tri, Vector3D* position, Vector3D* normal, double r1, double r2);

        SPICA_EXPORTS void onQuad(const Quad& quad, Vector3D* position, Vector3D* normal);

        SPICA_EXPORTS void onQuad(const Quad& quad, Vector3D* position, Vector3D* normal, double r1, double r2);

        SPICA_EXPORTS void on(const IShape* shape, Vector3D* position, Vector3D* normal);

        SPICA_EXPORTS void on(const IShape* shape, Vector3D* position, Vector3D* normal, double r1, double r2);

        SPICA_EXPORTS
        void poissonDisk(const std::vector<Triangle>& triangles,
                         const double minDist,
                         std::vector<Vector3D>* points,
                         std::vector<Vector3D>* normals);

    }  // namespace sampler

}  // namespace spica

#endif  // SPICA_SAMPLER_H_
