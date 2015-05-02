#ifndef _SPICA_BBOX_H_
#define _SPICA_BBOX_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifndef SPICA_BBOX_EXPORT
        #define SPICA_BBOX_DLL __declspec(dllexport)
    #else
        #define SPICA_BBOX_DLL __declspec(dllimport)
    #endif
#else
#define SPICA_BBOX_DLL
#endif

#include "../renderer/ray.h"
#include "../utils/vector3.h"

namespace spica {

    // ----------------------------------------
    // Axis-aligned bounding box
    // ----------------------------------------    
    class SPICA_BBOX_DLL BBox {
    private:
        Vector3 _posMin;    // Position of minimum corner
        Vector3 _posMax;    // Position of maximum corner
        
    public:
        BBox();
        explicit BBox(double minX, double minY, double minZ, double maxX, double maxY, double maxZ);
        explicit BBox(const Vector3& posMin, const Vector3& posMax);
        BBox(const BBox& box);

        ~BBox();

        BBox& operator=(const BBox& box);

        bool intersect(const Ray& ray, double* tMin, double* tMax) const;

		inline Vector3 posMin() const { return _posMin; }
		inline Vector3 posMax() const { return _posMax; }
    };

}  // namespace spica

#endif  // _SPICA_BBOX_H_
