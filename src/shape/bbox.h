#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BBOX_H_
#define _SPICA_BBOX_H_

#include "shape_interface.h"
#include "triangle.h"
#include "../renderer/ray.h"
#include "../math/vector3d.h"

namespace spica {

    /** AABB (Axis-Aligned Bounding Box) class
     *  @ingroup shape_module
     */
    class SPICA_EXPORTS BBox : public IShape {
    private:
        Vector3D _posMin;    // Position of minimum corner
        Vector3D _posMax;    // Position of maximum corner
        
    public:
        BBox();
        BBox(double minX, double minY, double minZ, double maxX, double maxY, double maxZ);
        BBox(const Vector3D& posMin, const Vector3D& posMax);
        BBox(const BBox& box);

        ~BBox();

        BBox& operator=(const BBox& box);

        bool intersect(const Ray& ray, double* tMin, double* tMax) const;

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

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

        //! Triangulate
        std::vector<Triangle> triangulate() const override;

        //! Total area
        double area() const override;

        inline Vector3D posMin() const { return _posMin; }
        inline Vector3D posMax() const { return _posMax; }
    };

}  // namespace spica

#endif  // _SPICA_BBOX_H_
