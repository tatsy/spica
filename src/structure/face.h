#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_STRUCTURE_FACE_H_
#define _SPICA_STRUCTURE_FACE_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifndef SPICA_STRUCTURE_FACE_EXPORT
        #define SPICA_STRUCTURE_FACE_DLL __declspec(dllexport)
    #else
        #define SPICA_STRUCTURE_FACE_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_STRUCTURE_FACE_DLL
#endif

#include <memory>
#include <vector>

namespace spica {

    namespace structure {

        class Vertex;
        class HalfEdge;
    
        class Face {
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
