#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_STRUCTURE_FACE_H_
#define _SPICA_STRUCTURE_FACE_H_

#include <memory>
#include <vector>

#include "../core/common.h"

namespace spica {

    namespace structure {

        class Vertex;
        class HalfEdge;
    
        class SPICA_EXPORTS Face {
        private:
            std::vector<std::weak_ptr<Vertex> >   _vertices;
            std::vector<std::weak_ptr<HalfEdge> > _halfedges;

        public:
            Face();
            Face(const Face& face);

            ~Face();

            Face& operator=(const Face& face);
        };

    }

}

#endif  // _SPICA_STRUCTURE_FACE_H_
