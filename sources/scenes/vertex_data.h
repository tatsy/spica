#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_VERTEX_DATA_H_
#define _SPICA_VERTEX_DATA_H_

#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../math/vector2d.h"
#include "../math/vector3d.h"
#include "../core/spectrum.h"

namespace spica {

    class VertexData {
    private:
        Point3d    _pos;
        Spectrum _color;
        Normal3d _normal;
        Vector2d _texcoord;
        bool     _isTextured;

    public:
        VertexData()
            : _pos()
            , _color()
            , _normal()
            , _texcoord(INFTY, INFTY)
            , _isTextured(false) {
        }

        explicit VertexData(const Point3d& pos,
                            const Spectrum& color = Spectrum(),
                            const Normal3d& normal = Normal3d(0.0, 0.0, 0.0),
                            const Vector2d& texcoord = Vector2d(INFTY, INFTY))
            : _pos(pos)
            , _color(color)
            , _normal(normal)
            , _texcoord(texcoord)
            , _isTextured(false) {
            if (0.0 <= texcoord.x() && 0.0 <= texcoord.y() &&
                texcoord.x() <= 1.0 && texcoord.y() <= 1.0) {
                _isTextured = true;
            }
        }

        VertexData(const VertexData& v) 
            : _pos(v._pos)
            , _color(v._color)
            , _normal(v._normal)
            , _texcoord(v._texcoord)
            , _isTextured(v._isTextured) {
        }

        ~VertexData() {
        }
        
        VertexData& operator=(const VertexData& v) {
            this->_pos      = v._pos;
            this->_color    = v._color;
            this->_normal   = v._normal;
            this->_texcoord = v._texcoord;
            this->_isTextured = v._isTextured;
            return *this;
        }

        inline const Point3d& pos() const { return _pos; }
        inline const Spectrum& color() const { return _color; }
        inline const Normal3d& normal() const { return _normal; }
        inline const Vector2d& texcoord() const { return _texcoord; }
        inline bool isTextured() const { return _isTextured; }

        inline void setPosition(const Point3d& pos) { _pos = pos; }
        inline void setColor(const Spectrum& color) { _color = color; }
        inline void setNormal(const Normal3d& nrm) { _normal = nrm; }
        inline void setTexcoord(const Vector2d& texcoord) { _texcoord = texcoord; }
    };

}  // namespace spica

#endif  // _SPICA_VERTEX_DATA_H_
