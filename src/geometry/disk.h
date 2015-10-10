#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_DISK_H_
#define _SPICA_DISK_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_DISK_EXPORT
        #define SPICA_DISK_DLL __declspec(dllexport)
    #else
        #define SPICA_DISK_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_DISK_DLL
#endif

#include "../math/vector3d.h"
#include "geometry_interface.h"

namespace spica {

    class SPICA_DISK_DLL Disk : public IGeometry {    
    private:
        Vector3D _center;
        Vector3D _normal;
        double _radius;

    public:
        Disk();
        Disk(const Vector3D& center, const Vector3D& normal, double radius);
        Disk(const Disk& disk);
        ~Disk();

        Disk& operator=(const Disk& disk);

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const;

        double area() const;

        std::vector<Triangle> triangulate() const;

        inline Vector3D center() const { return _center; }
        inline Vector3D normal() const { return _normal; }
        inline double  radius() const { return _radius; }
    
    };

}  // namespace spica

#endif  // _SPICA_DISK_H_
