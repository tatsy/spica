#define SPICA_API_EXPORT
#include "vertex.h"
#include "face.h"
#include "halfedge.h"

namespace spica {

    namespace structure {
    
        Vertex::Vertex()
            : Vector3D()
            , _faces()
            , _halfedges() {
        }

        Vertex::Vertex(const Vector3D& v)
            : Vector3D(v)
            , _faces()
            , _halfedges() {
        }

        Vertex::Vertex(const Vertex& v)
            : Vector3D()
            , _faces()
            , _halfedges() {
            this->operator=(v);
        }

        Vertex::~Vertex() {
        }

        Vertex& Vertex::operator=(const Vertex& v) {
            Vector3D::operator=(v);
            _faces     = v._faces;
            _halfedges = v._halfedges;
            return *this;
        }

    }  // namespace structure

}  // namespace spica