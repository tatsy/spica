#ifndef _SPICA_FORWARD_DECL_H_
#define _SPICA_FORWARD_DECL_H_

#include <type_traits>

namespace spica {

    // Core module
    class RGBSpectrum;
    class VertexData;
    class TriangleData;

    class Primitive;
    class GeometricPrimitive;

    template <class T>
    class Stack;

    class MemoryArena;

    template <class T, class Enable>
    class Rect_;
    using Rect = Rect_<int, void>;
    using RectF = Rect_<double, void>;

    template <class T>
    class Bound3_;
    using Bound3i = Bound3_<int>;
    using Bound3f = Bound3_<float>;
    using Bound3d = Bound3_<double>;

    // Image module
    class Image;
    class MipMap;

    // Math module
    template <class T, class Enable>
    class Vector2_;
    using Vector2D = Vector2_<double, void>;

    template <class T>
    class Vector3_;
    using Vector3D = Vector3_<double>;

    template <class T>
    class Point2_;
    using Point2D = Point2_<double>;

    template <class T>
    class Point3_;
    using Point3D = Point3_<double>;
    using Point = Point3D;

    template <class T>
    class Normal3_;
    using Normal3D = Normal3_<double>;
    using Normal = Normal3D;

    class Matrix4x4;
    class Transform;

    // Scene
    class Scene;

    // Camera module
    class Camera;
    class DoFCamera;
    class OrthographicCamera;
    class PerspectiveCamera;

    // Renderer module
    class Ray;
    class Interaction;
    class SurfaceInteraction;
    class RenderParameters;
    class Photon;
    class SubsurfaceIntegrator;

    // Accelerator module
    enum class AccelType;
    class AccelInterface;
    class KdTreeAccel;
    class QBVHAccel;
    
    // Shape module
    class Disk;
    class Plane;
    class Quad;
    class Shape;
    class Sphere;
    class Triangle;
    class Trimesh;

    // Light module
    enum class LightType;
    class Light;
    class AreaLight;
    class Envmap;
    class LightSample;
    class VisibilityTester;

    // Material module
    class Material;

    // Texture module
    template <class T>
    class Texture;

    // BSDF module
    class BSDF;
    class BSSRDF;

    // Random module
    class Random;
    class Halton;
    class Sampler;

}  // namespace spica

#endif  // _SPICA_FORWARD_DECL_H_
