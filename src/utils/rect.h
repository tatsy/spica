#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RECT_H_
#define _SPICA_RECT_H_

#include "common.h"

#ifdef WITH_ENABLER
extern void* enabler;
#endif

namespace spica {

    #ifdef WITH_ENABLER
    template <class T,
              typename
              std::enable_if<std::is_arithmetic<T>::value>::type *& = enabler>
    #else
    template <class T>
    #endif
    class Rect_;

    template <class T>
    class Rect_<T> {
    private:
        T _x, _y, _width, _height;

    public:
        Rect_()
            : _x(0)
            , _y(0)
            , _width(0)
            , _height(0) {
        }

        Rect_(T x, T y, T width, T height)
            : _x(x)
            , _y(y)
            , _width(width)
            , _height(height) {
        }

        Rect_(const Rect_& rect) 
            : _x(rect._x)
            , _y(rect._y)
            , _width(rect._width)
            , _height(rect._height) {
        }

        ~Rect_() {
        }

        Rect_& operator=(const Rect_& rect) {
            this->_x = rect._x;
            this->_y = rect._y;
            this->_width = rect._width;
            this->_height = rect._height;
            return *this;
        }

        inline T x() const { return _x; }
        inline T y() const { return _y; }
        inline T width() const { return _width; }
        inline T height() const { return _height; }
    };

    using Rect  = Rect_<int>;
    using RectF = Rect_<double>;

}  // namespace spica

#endif  // _SPICA_RECT_H_
