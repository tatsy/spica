#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_STRUCTURE_MESH_H_
#define _SPICA_STRUCTURE_MESH_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_STRUCTURE_MESH_EXPORT
        #define SPICA_STRUCTURE_MESH_DLL __declspec(dllexport)
    #else
        #define SPICA_STRUCTURE_MESH_DLL __declspec(dllexport)
    #endif
#else
    #define SPICA_STRUCTURE_MESH_DLL
#endif

#include <vector>

#include "../geometry/trimesh.h"
#include "vertex.h"
#include "face.h"
#include "halfedge.h"

namespace spica {

    namespace structure {
    
        class SPICA_STRUCTURE_MESH_DLL Mesh {
        private:
            std::vector<Vertex>   _vertices;
            std::vector<Face>     _faces;
            std::vector<HalfEdge> _halfedges;

        public:
            Mesh();
            Mesh(const Mesh& mesh);
            Mesh(Mesh&& mesh);
            
            ~Mesh();

            Mesh& operator=(const Mesh& mesh);
            Mesh& operator=(Mesh&& mesh);

            void load(const std::string& filename);

            static Mesh fromFile(const std::string& filename);
            static Mesh fromTMesh(const Trimesh& trimesh);

            inline int numVertes() const { return static_cast<int>(_vertices.size()); }
            inline int numFaces()  const { return static_cast<int>(_faces.size()); }
        };

    }

}  // namespace spica

#endif  // _SPICA_STRUCTURE_MESH_H_