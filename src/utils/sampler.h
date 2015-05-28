#ifndef SPICA_SAMPLER_H_
#define SPICA_SAMPLER_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_SAMPLER_EXPORT
        #define SPICA_SAMPLER_DLL __declspec(dllexport)
    #else
        #define SPICA_SAMPLER_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_SAMPLER_DLL
#endif

#include "vector3.h"
#include "../geometry/geometry.h"
#include "../random/random.h"

namespace spica {

    namespace sampler {

        SPICA_SAMPLER_DLL void onHemisphere(const Vector3& normal, Vector3* direction);

        SPICA_SAMPLER_DLL void onHemisphere(const Vector3& normal, Vector3* direction, double r1, double r2);

        SPICA_SAMPLER_DLL void onSphere(const Sphere& sphere, Vector3* position, Vector3* normal);
    
        SPICA_SAMPLER_DLL void onDisk(const Disk& disk, Vector3* position, Vector3* normal);

        SPICA_SAMPLER_DLL void onTriangle(const Triangle& tri, Vector3* position, Vector3* normal);

        SPICA_SAMPLER_DLL void onQuad(const Quad& quad, Vector3* position, Vector3* normal);

        SPICA_SAMPLER_DLL void onQuad(const Quad& quad, Vector3* position, Vector3* normal, double r1, double r2);

        SPICA_SAMPLER_DLL void on(const Primitive* primitive, Vector3* position, Vector3* normal);

        SPICA_SAMPLER_DLL void on(const Primitive* primitive, Vector3* position, Vector3* normal, double r1, double r2);

        SPICA_SAMPLER_DLL void poissonDisk(const Trimesh& trimesh, const double minDist, std::vector<Vector3>* points, std::vector<Vector3>* normals);

    
    }  // namespace sampler

}  // namespace spica

#endif  // SPICA_SAMPLER_H_
