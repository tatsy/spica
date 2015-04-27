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

#include "../utils/vector3.h"
#include "triangle.h"
#include "primitive.h"

namespace spica {

    class SPICA_TRIMESH_DLL Trimesh : public Primitive {
    private:
        unsigned long _numVerts;
        unsigned long _numFaces;
        Vector3* _vertices;
        int* _faces;
        Vector3* _normals;

    public:
        Trimesh();
        Trimesh(const std::string& filename, const Material& material = Material());
        Trimesh(const Trimesh& trimesh);
        virtual ~Trimesh();

        Trimesh& operator=(const Trimesh& trimesh);

        bool intersect(const Ray& ray, Intersection& intersect) const;

        void load(const std::string& filename);

        Triangle getTriangle(int id) const;
        Vector3 getNormal(int id) const;

        inline unsigned long numVerts() const { return _numVerts; }
        inline unsigned long numFaces() const { return _numFaces; }
    };

}  // namespace spica

#endif  // _SPICA_TRIMESH_H_