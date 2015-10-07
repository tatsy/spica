#define SPICA_STRUCTURE_MESH_EXPORT
#include "mesh.h"

namespace spica {

    namespace structure {
    
        Mesh::Mesh()
            : _vertices()
            , _faces()
            , _halfedges() {
        }

        Mesh::Mesh(const Mesh& mesh)
            : _vertices()
            , _faces()
            , _halfedges() {
            this->operator=(mesh);
        }

        Mesh::Mesh(Mesh&& mesh)
            : _vertices()
            , _faces()
            , _halfedges() {
            this->operator=(std::move(mesh));
        }

        Mesh::~Mesh() {
        }

        Mesh& Mesh::operator=(const Mesh& mesh) {
            this->_vertices  = mesh._vertices;
            this->_faces     = mesh._faces;
            this->_halfedges = mesh._halfedges;
            return *this;
        }

        Mesh& Mesh::operator=(Mesh&& mesh) {
            this->_vertices  = std::move(mesh._vertices);
            this->_faces     = std::move(mesh._faces);
            this->_halfedges = std::move(mesh._halfedges);
            return *this;
        }

        void Mesh::load(const std::string& filename) {
            (*this) = Mesh::fromFile(filename);            
        }

        Mesh Mesh::fromFile(const std::string& filename) {
            Trimesh trimesh(filename);
            return Mesh::fromTMesh(trimesh);
        }

        Mesh Mesh::fromTMesh(const Trimesh& trimesh) {
            Mesh mesh;

            // Copy vertices
            for (int i = 0; i < trimesh.numVerts(); i++) {
                Vertex v(trimesh.getVertex(i));
                mesh._vertices.emplace_back(v);
            }
            return std::move(mesh);
        }

    }  // namespace structure

}  // namespace spica