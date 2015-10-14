#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_STRUCTURE_HALFEDGE_H_
#define _SPICA_STRUCTURE_HALFEDGE_H_

#include <memory>

#include "../core/common.h"

namespace spica {

    namespace structure {

        class Vertex;
        class Face;
    
        class SPICA_EXPORTS HalfEdge {
        private:
            std::weak_ptr<Face>   _face;
            std::weak_ptr<Vertex> _from;
            std::weak_ptr<Vertex> _to;

        public:
            HalfEdge();
            ~HalfEdge();
        };

    }  //namespace structure

}  // namespace spica

#endif  // _SPICA_STRUCTURE_HALFEDGE_H_
