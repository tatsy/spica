#ifndef SPICA_SAMPLER_H_
#define SPICA_SAMPLER_H_

#include "common.h"
#include "../math/vector3d.h"
#include "../shape/shape.h"
#include "../random/random.h"

namespace spica {

    namespace sampler {

        SPICA_EXPORTS void onHemisphere(const Normal& normal, Vector3D* direction);

        SPICA_EXPORTS void onHemisphere(const Normal& normal, Vector3D* direction, double r1, double r2);

        SPICA_EXPORTS void onSphere(const Sphere& sphere, Point* position, Normal* normal);

        SPICA_EXPORTS void onSphere(const Sphere& sphere, Point* position, Normal* normal, double r1, double r2);
    
        SPICA_EXPORTS void onDisk(const Disk& disk, Point* position, Normal* normal);

        SPICA_EXPORTS void onTriangle(const Triangle& tri, Point* position, Normal* normal);

        SPICA_EXPORTS void onTriangle(const Triangle& tri, Point* position, Normal* normal, double r1, double r2);

        SPICA_EXPORTS void onQuad(const Quad& quad, Point* position, Normal* normal);

        SPICA_EXPORTS void onQuad(const Quad& quad, Point* position, Normal* normal, double r1, double r2);

        SPICA_EXPORTS void on(const IShape* shape, Point* position, Normal* normal);

        SPICA_EXPORTS void on(const IShape* shape, Point* position, Normal* normal, double r1, double r2);

        SPICA_EXPORTS
        void poissonDisk(const std::vector<Triangle>& triangles,
                         const double minDist,
                         std::vector<Point>* points,
                         std::vector<Normal>* normals);

    }  // namespace sampler

}  // namespace spica

#endif  // SPICA_SAMPLER_H_
