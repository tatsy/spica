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

#include "../renderer/material.h"
#include "../geometry/geometry.h"

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

        void add(const Primitive* p, const Color& color);
        void add(const Vector3& v, const Vector3& normal, const Color& color);
        void add(const Quad& quad, const Color& color);
        void add(const Triangle& tri, const Color& color);
        void add(const Trimesh& trimesh, const Color& color);
        void add(const Sphere& sphere, const Color& color);

        inline int numIndices() const { return (int)_indices.size(); }

        inline const float* vertices() const { return &_vertices[0]; }
        inline const float* normals() const { return &_normals[0]; }
        inline const float* colors() const { return &_colors[0]; }
        inline const unsigned int* indices() const { return &_indices[0]; }
    };

}

#endif  // _SPICA_VBO_H_
