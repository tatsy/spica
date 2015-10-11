#ifndef _SPICA_FORWARD_DECL_H_
#define _SPICA_FORWARD_DECL_H_

#include <type_traits>

namespace spica {

    // Core module
    class Color;
    class Image;
    class VertexData;
    class TriangleData;

    template <class T>
    class Stack;

    // Math module
    template <class T, class Enable>
    class Vector2_;
    using Vector2D = Vector2_<double, void>;

    template <class T, class Enable>
    class Vector3_;
    using Vector3D = Vector3_<double, void>;

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
    class SubsurfaceIntegrator;

    // Accelerator module
    enum class AccelType;
    class AccelBase;
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
    class Lighting;
    class LightSample;

    // BSDF module
    class BSDF;
    class BSSRDF;

}  // namespace spica

#endif  // _SPICA_FORWARD_DECL_H_
