#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_STRUCTURE_VERTEX_H_
#define _SPICA_STRUCTURE_VERTEX_H_

#include <vector>
#include <memory>

#include "../math/vector3d.h"

namespace spica {

    namespace structure {

        class Face;
        class HalfEdge;
    
        class SPICA_EXPORTS Vertex : public Vector3D {
        private:
            std::vector<std::weak_ptr<Face> >     _faces;
            std::vector<std::weak_ptr<HalfEdge> > _halfedges;

        public:
            Vertex();
            Vertex(const Vector3D& v);
            Vertex(const Vertex& v);

            ~Vertex() override;

            Vertex& operator=(const Vertex& v);
        };

    }  // namespace spica

}  // namespace spica

#endif  // _SPICA_STRUCTURE_VERTEX_H_
