#ifndef _SPICA_FORWARD_DECL_H_
#define _SPICA_FORWARD_DECL_H_

#include <type_traits>

namespace spica {

    // Core module
    class RGBSpectrum;
    class Image;
    class VertexData;
    class TriangleData;

    template <class T>
    class Stack;

    template <class T, class Enable>
    class Rect_;
    using Rect = Rect_<int, void>;
    using RectF = Rect_<double, void>;

    // Math module
    template <class T, class Enable>
    class Vector2_;
    using Vector2D = Vector2_<double, void>;

    template <class T>
    class Vector3_;
    using Vector3D = Vector3_<double>;

    template <class T>
    class Point3_;
    using Point3D = Point3_<double>;
    using Point = Point3D;

    template <class T>
    class Normal3_;
    using Normal3D = Normal3_<double>;
    using Normal = Normal3D;

    // Scene
    class Scene;

    // Camera module
    class Camera;
    class DoFCamera;
    class OrthographicCamera;
    class PerspectiveCamera;

    // Renderer module
    class Ray;
    class Hitpoint;
    class Intersection;
    class RenderParameters;
    class Photon;
    class SubsurfaceIntegrator;

    // Accelerator module
    enum class AccelType;
    class IAccel;
    class KdTreeAccel;
    class QBVHAccel;
    
    // Shape module
    class BBox;
    class Disk;
    class Plane;
    class Quad;
    class Shape;
    class Sphere;
    class Triangle;
    class Trimesh;

    // Lighting module
    enum class LightType;
    class Lighting;
    class LightSample;

    // BSDF module
    class BSDF;
    class BSSRDF;

    // Random module
    class Random;
    class Halton;
    class RandomSampler;

}  // namespace spica

#endif  // _SPICA_FORWARD_DECL_H_
