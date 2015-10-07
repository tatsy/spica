#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_STRUCTURE_HALFEDGE_H_
#define _SPICA_STRUCTURE_HALFEDGE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_STRUCTURE_HALFEDGE_EXPORT
        #define SPICA_STRUCTURE_HALFEDGE_DLL __declspec(dllexport)
    #else
        #define SPICA_STRUCTURE_HALFEDGE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_STRUCTURE_HALFEDGE_DLL
#endif

#include <memory>

namespace spica {

    namespace structure {

        class Vertex;
        class Face;
    
        class HalfEdge {
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
