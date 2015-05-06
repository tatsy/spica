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

#include "../utils/vector3.h"
#include "primitive.h"

namespace spica {

    class SPICA_DISK_DLL Disk : public Primitive {    
    private:
        Vector3 _center;
        Vector3 _normal;
        double _radius;

    public:
        Disk();
        Disk(const Vector3& center, const Vector3& normal, double radius);
        Disk(const Disk& disk);
        ~Disk();

        Disk& operator=(const Disk& disk);

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const;

        double area() const;

        inline Vector3 center() const { return _center; }
        inline Vector3 normal() const { return _normal; }
        inline double  radius() const { return _radius; }
    
    };

}  // namespace spica

#endif  // _SPICA_DISK_H_
