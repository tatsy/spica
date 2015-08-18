#ifndef SPICA_SAMPLER_H_
#define SPICA_SAMPLER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SAMPLER_EXPORT
        #define SPICA_SAMPLER_DLL __declspec(dllexport)
    #else
        #define SPICA_SAMPLER_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_SAMPLER_DLL
#endif

#include "vector3d.h"
#include "../geometry/geometry.h"
#include "../random/random.h"

namespace spica {

    namespace sampler {

        SPICA_SAMPLER_DLL void onHemisphere(const Vector3D& normal, Vector3D* direction);

        SPICA_SAMPLER_DLL void onHemisphere(const Vector3D& normal, Vector3D* direction, double r1, double r2);

        SPICA_SAMPLER_DLL void onSphere(const Sphere& sphere, Vector3D* position, Vector3D* normal);

        SPICA_SAMPLER_DLL void onSphere(const Sphere& sphere, Vector3D* position, Vector3D* normal, double r1, double r2);
    
        SPICA_SAMPLER_DLL void onDisk(const Disk& disk, Vector3D* position, Vector3D* normal);

        SPICA_SAMPLER_DLL void onTriangle(const Triangle& tri, Vector3D* position, Vector3D* normal);

        SPICA_SAMPLER_DLL void onTriangle(const Triangle& tri, Vector3D* position, Vector3D* normal, double r1, double r2);

        SPICA_SAMPLER_DLL void onQuad(const Quad& quad, Vector3D* position, Vector3D* normal);

        SPICA_SAMPLER_DLL void onQuad(const Quad& quad, Vector3D* position, Vector3D* normal, double r1, double r2);

        SPICA_SAMPLER_DLL void on(const IGeometry* primitive, Vector3D* position, Vector3D* normal);

        SPICA_SAMPLER_DLL void on(const IGeometry* primitive, Vector3D* position, Vector3D* normal, double r1, double r2);

        SPICA_SAMPLER_DLL void poissonDisk(const Trimesh& trimesh, const double minDist, std::vector<Vector3D>* points, std::vector<Vector3D>* normals);

    }  // namespace sampler

}  // namespace spica

#endif  // SPICA_SAMPLER_H_
