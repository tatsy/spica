#define SPICA_API_EXPORT
#include "face.h"
#include "vertex.h"
#include "halfedge.h"

namespace spica {

    namespace structure {
    
        Face::Face()
            : _vertices()
            , _halfedges() {
        }

        Face::Face(const Face& face)
            : _vertices()
            , _halfedges() {
        }

        Face::~Face() {
        }

        Face& Face::operator=(const Face& face) {
            this->_vertices  = face._vertices;
            this->_halfedges = face._halfedges;
            return *this;
        }

    }  // namespace structure

}  // namespace spica
