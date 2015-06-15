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
#include <memory>

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
        std::shared_ptr<AccelBase> _accel;
        AccelType _accelType;

    public:
        // Contructor
        Trimesh();

        // Constructor
        // @param[in] filename: .ply or .obj file
        Trimesh(const std::string& filename);

        // Constructor
        // @param[in] vertices: vertices to form trimesh
        // @param[in] faceIDs: face IDs (stating from 0)
        Trimesh(const std::vector<Vector3>& vertices, const std::vector<int>& faceIDs);

        // Copy constructor
        Trimesh(const Trimesh& trimesh);

        virtual ~Trimesh();

        Trimesh& operator=(const Trimesh& trimesh);

        // Check intersection
        // @param[in] ray: input to check intersect with
        // @param[out] hitpoint: hit point if intersected
        bool intersect(const Ray& ray, Hitpoint* hitpoint) const;

        // Area of trimesh
        double area() const;

        // Set an intersection accelerator type
        // @param[in] accelType: QBVH or k-d tree
        // @param[in] doBuild: construct the accelerator immediately after this function call or not
        void setAccelType(AccelType accelType, bool doBuild = false);

        // Load mesh file
        // @param[in] filename: .obj or .ply file
        void load(const std::string& filename);

        // Translate the mesh
        // @param[in] transtion delta
        void translate(const Vector3& move);

        // Scale the mesh
        // @param[in] scaleX: scaling along X-axis
        // @param[in] scaleY: scaling along Y-axis
        // @param[in] scaleZ: scaling along Z-axis
        void scale(const double scaleX, const double scaleY, const double scaleZ);

        // Put the mesh on the specified plane
        // @param[in] plane: a plane on which the mesh is put
        void putOnPlane(const Plane& plane);

        // Get a triangle with specified ID
        // @param[in] faceID: ID of the triangle to get
        Triangle getTriangle(int faceID) const;

        // Get the normal of a face with specified ID
        // @param[in] faceID: ID of the triangle to get a normal
        Vector3 getNormal(int faceID) const;

        // Get the number of vertices
        inline unsigned long numVerts() const { return _numVerts; }

        // Get the number of faces (triangles)
        inline unsigned long numFaces() const { return _numFaces; }

    private:
        void release();
        void buildAccel();

        void loadPly(const std::string& filename);
        void loadObj(const std::string& filename);
    };

}  // namespace spica

#endif  // _SPICA_TRIMESH_H_
