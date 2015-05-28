#ifndef _SPICA_TRIMESH_H_
#define _SPICA_TRIMESH_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_TRIMESH_EXPORT
        #define SPICA_TRIMESH_DLL __declspec(dllexport)
    #else
        #define SPICA_TRIMESH_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_TRIMESH_DLL
#endif

#include <string>

#include "primitive.h"
#include "triangle.h"
#include "plane.h"
#include "../utils/vector3.h"
#include "../accel/accel.h"

namespace spica {

    class SPICA_TRIMESH_DLL Trimesh : public Primitive {
    private:
        unsigned long _numVerts;
        unsigned long _numFaces;
        Vector3* _vertices;
        int* _faces;
        Vector3* _normals;
        AccelBase* _accel;
        AccelType _accelType;

    public:
        Trimesh();
        Trimesh(const std::string& filename);
        Trimesh(const Trimesh& trimesh);
        virtual ~Trimesh();

        Trimesh& operator=(const Trimesh& trimesh);

        bool intersect(const Ray& ray, Hitpoint* hitpoint) const;

        double area() const;

        void setAccelType(AccelType accelType, bool doBuild = false);

        void load(const std::string& filename);

        void translate(const Vector3& move);
        void scale(const double scaleX, const double scaleY, const double scaleZ);
        void putOnPlane(const Plane& plane);

        Triangle getTriangle(int id) const;
        Vector3 getNormal(int id) const;

        inline unsigned long numVerts() const { return _numVerts; }
        inline unsigned long numFaces() const { return _numFaces; }

    private:
        void buildAccel();
    };

}  // namespace spica

#endif  // _SPICA_TRIMESH_H_