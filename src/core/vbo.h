#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VBO_H_
#define _SPICA_VBO_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_VBO_EXPORT
        #define SPICA_VBO_DLL __declspec(dllexport)
    #else
        #define SPICA_VBO_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_VBO_DLL
#endif

#include <vector>
#include <type_traits>

#include "../geometry/geometry.h"

extern void* enabler;

namespace spica {

    class SPICA_VBO_DLL VBO {
    private:
        std::vector<float> _vertices;
        std::vector<float> _normals;
        std::vector<float> _colors;
        std::vector<unsigned int> _indices;

    public:
        VBO();
        VBO(const VBO& vbo);
        ~VBO();

        VBO& operator=(const VBO& vbo);

        void add(const Vector3D& v, const Vector3D& normal, const Color& color);

        template <class T, typename std::enable_if<std::is_base_of<IGeometry, T>::value>::type *& = enabler>
        void add(const T& shape, const Color& color);

        inline int numIndices() const { return (int)_indices.size(); }

        inline const float* vertices() const { return &_vertices[0]; }
        inline const float* normals() const { return &_normals[0]; }
        inline const float* colors() const { return &_colors[0]; }
        inline const unsigned int* indices() const { return &_indices[0]; }
    };

    template <class T, typename std::enable_if<std::is_base_of<IGeometry, T>::value>::type *&>
    void VBO::add(const T& shape, const Color& color) {        
        const std::vector<Triangle> tris = shape.triangulate();
        for (int i = 0; i < tris.size(); i++) {
            const Vector3D& normal = tris[i].normal();
            for (int j = 0; j < 3; j++) {
                add(tris[i][j], normal, color);
            }
        }
    }
}

#endif  // _SPICA_VBO_H_
