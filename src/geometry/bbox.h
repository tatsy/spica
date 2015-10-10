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

#include "triangle.h"
#include "../renderer/ray.h"
#include "../math/vector3d.h"

namespace spica {

    // ----------------------------------------
    // Axis-aligned bounding box
    // ----------------------------------------    
    class SPICA_BBOX_DLL BBox {
    private:
        Vector3D _posMin;    // Position of minimum corner
        Vector3D _posMax;    // Position of maximum corner
        
    public:
        BBox();
        explicit BBox(double minX, double minY, double minZ, double maxX, double maxY, double maxZ);
        explicit BBox(const Vector3D& posMin, const Vector3D& posMax);
        BBox(const BBox& box);

        ~BBox();

        BBox& operator=(const BBox& box);

        bool intersect(const Ray& ray, double* tMin, double* tMax) const;

        //! Make BBox from Triangle
        static BBox fromTriangle(const Triangle& t);

        // Enlarge box to contain vertex or box
        void merge(const Vector3D& v);
        void merge(const BBox& box);
        void merge(const Triangle& t);
        static BBox merge(const BBox& b1, const BBox& b2);

        // Check if vertex is inside or not
        bool inside(const Vector3D& v) const;

        //! Maximum extent: returns 0 -> x, 1 -> y, 2 -> z
        int maximumExtent() const;

        //! Total area
        double area() const;

        inline Vector3D posMin() const { return _posMin; }
        inline Vector3D posMax() const { return _posMax; }
    };

}  // namespace spica

#endif  // _SPICA_BBOX_H_
