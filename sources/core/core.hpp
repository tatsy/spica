#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CORE_HPP_
#define _SPICA_CORE_HPP_

namespace spica {

// Core module
class RGBSpectrum;
class VertexData;
class TriangleData;

class Accelerator;
class Primitive;
class GeometricPrimitive;

class Distribution1D;
class Distribution2D;

class CatmullRom;
class CatmullRom2D;

template <class T>
class Stack;

class MemoryArena;

template <class T>
class Bounds2_;
using Bounds2i = Bounds2_<int>;
using Bounds2f = Bounds2_<float>;
using Bounds2d = Bounds2_<double>;

template <class T>
class Bounds3_;
using Bounds3i = Bounds3_<int>;
using Bounds3f = Bounds3_<float>;
using Bounds3d = Bounds3_<double>;

// Image module
class Image;
class Film;
class MipMap;
enum class ImageWrap;

// Math module
template <class T>
class Vector2_;
using Vector2i = Vector2_<int>;
using Vector2f = Vector2_<float>;
using Vector2d = Vector2_<double>;

template <class T>
class Vector3_;
using Vector3i = Vector3_<int>;
using Vector3f = Vector3_<float>;
using Vector3d = Vector3_<double>;

template <class T>
class Point2_;
using Point2i = Point2_<int>;
using Point2f = Point2_<float>;
using Point2d = Point2_<double>;

template <class T>
class Point3_;
using Point3i = Point3_<int>;
using Point3f = Point3_<float>;
using Point3d = Point3_<double>;

template <class T>
class Normal3_;
using Normal3f = Normal3_<float>;
using Normal3d = Normal3_<double>;

class Matrix4x4;
class Transform;

class Ray;

}  // namespace spica

#endif  // _SPICA_CORE_H_
