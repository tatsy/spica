#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VERTEX_DATA_H_
#define _SPICA_VERTEX_DATA_H_

#include "vector2d.h"
#include "vector3d.h"
#include "color.h"

namespace spica {

    class VertexData {
    private:
        Vector3D _pos;
        Color    _color;
        Vector3D _normal;
        Vector2D _texcoord;

    public:
        VertexData()
            : _pos()
            , _color()
            , _normal()
            , _texcoord() {
        }

        VertexData(const VertexData& v) 
            : _pos(v._pos)
            , _color(v._color)
            , _normal(v._normal)
            , _texcoord(v._texcoord) {
        }

        ~VertexData() {
        }
        
        VertexData& operator=(const VertexData& v) {
            this->_pos      = v._pos;
            this->_color    = v._color;
            this->_normal   = v._normal;
            this->_texcoord = v._texcoord;
            return *this;
        }

        inline Vector3D pos() const { return _pos; }
        inline Color color() const { return _color; }
        inline Vector3D normal() const { return _normal; }
        inline Vector2D texcoord() const { return _texcoord; }
    };

}  // namespace spica

#endif  // _SPICA_VERTEX_DATA_H_
