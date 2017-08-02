#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_HPP_
#define _SPICA_RENDER_HPP_

namespace spica {

// Plugin classes
class Integrator;
class Camera;
class Medium;
class MediumInterface;
class SurfaceMaterial;
class SubsurfaceMaterial;

// Scene
class Scene;

// Camera module
class Camera;

// Renderer module
class Interaction;
class SurfaceInteraction;
class MediumInteraction;
class RenderParams;
class Photon;

class Integrator;
class HierarchicalIntegrator;

// Accelerator module
enum class AccelType;
class AccelInterface;
    
// Shape module
class Disk;
class Shape;
class Sphere;
class Triangle;

// Light module
enum class LightType;
class Light;
class LightSample;
class VisibilityTester;

// Material module
class Material;

// Medium module
class Medium;
class MediumInterface;
class PhaseFunction;

// Texture module
template <class T>
class Texture;

// BSDF module
class BSDF;
class BxDF;
class Fresnel;
class FresnelDielectric;
class MicrofacetDistribution;
class BSSRDF;
class DiffuseBSSRDF;
class DiffusionReflectance;

// Random module
class Random;
class Halton;
class Sampler;

}  // namespace spica

#endif  // _SPICA_RENDER_HPP_
