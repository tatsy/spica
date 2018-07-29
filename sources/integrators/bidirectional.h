#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_BIDIRECTIONAL_H_
#define _SPICA_BIDIRECTIONAL_H_

#include <variant>

#include "core/common.h"
#include "core/interaction.h"
#include "core/integrator.h"
#include "core/primitive.h"
#include "core/light.h"

namespace spica {

enum class VertexType : int {
    Camera, Light, Surface, Medium
};

double densityIBL(const Scene& scene, const Distribution1D& lightDist, const Vector3d& w);

template <class Type>
class ScopedAssignment {
public:
    ScopedAssignment(Type* target_ = nullptr, Type value = Type())
            : target{ target_ } {
        if (target) {
            backup = *target;
            *target = value;
        }
    }

    ~ScopedAssignment() {
        if (target) *target = backup;
    }

    ScopedAssignment(const ScopedAssignment& sa) = delete;
    ScopedAssignment& operator=(const ScopedAssignment& sa) = delete;

    ScopedAssignment& operator=(ScopedAssignment&& sa) {
        this->target = sa.target;
        this->backup = sa.backup;
        sa.target = nullptr;
        return *this;
    }

private:
    Type* target, backup;
};

struct EndpointInteraction : public Interaction {
    union {
        const Camera* camera;
        const Light* light;
    };

    EndpointInteraction()
            : Interaction{}
            , light{ nullptr } {
    }

    EndpointInteraction(const Ray& ray)
            : Interaction { ray.proceeded(1.0) }
            , light{ nullptr } {
        normal_ = Normal3d(-ray.dir());
    }

    EndpointInteraction(const Interaction& it, const Light* light_)
            : Interaction{ it }
            , light{ light_ } {
    }

    EndpointInteraction(const Interaction& it, const Camera* camera_)
            : Interaction{ it }
            , camera{ camera_ } {
    }

    EndpointInteraction(const Camera* camera_, const Ray& ray)
            : Interaction{ ray.org() }
            , camera{ camera_ } {
    }

    // TODO: a bit different.
    EndpointInteraction(const Light* light_, const Ray& r, const Normal3d& nl)
            : Interaction{ r.org() }
            , light{ light_ } {
        normal_ = nl;
    }
};

struct Vertex {
    // Public methods
    Vertex();
    Vertex(VertexType type_, const EndpointInteraction& ei, const Spectrum& beta_);
    Vertex(const SurfaceInteraction& it, const Spectrum& beta_);
    Vertex(const MediumInteraction& it, const Spectrum& beta_);

    Vertex(const Vertex& v);
    Vertex& operator=(const Vertex& v);

    Spectrum Le(const Scene& scene, const Vertex& v) const;
    Spectrum f(const Vertex& next) const;

    double convertDensity(double pdf, const Vertex& next) const;

    double pdf(const Scene& scene, const Vertex* prev, const Vertex& next) const;
    double pdfLight(const Scene& scene, const Vertex& v) const;
    double pdfLightOrigin(const Scene& scene, const Vertex& v, const Distribution1D& lightDist) const;

    bool isConnectible() const;

    // Public inline methods

    inline Point3d pos() const {
        return getInteraction().pos();
    }

    inline Normal3d normal() const {
        return getInteraction().normal();
    }

    inline Normal3d ns() const {
        if (type == VertexType::Surface) {
            return si()->ns();
        }
        return getInteraction().normal();
    }

    inline const EndpointInteraction * const ei() const {
        return std::get_if<EndpointInteraction>(&intr);
    }

    inline const SurfaceInteraction * const si() const {
        return std::get_if<SurfaceInteraction>(&intr);
    }

    inline const MediumInteraction * const mi() const {
        return std::get_if<MediumInteraction>(&intr);
    }

    inline const Interaction &getInteraction() const {
        switch (type) {
        case VertexType::Medium:
            return *mi();

        case VertexType::Surface:
            return *si();

        default:
            return *ei();
        }
    }

    inline bool isOnSurface() const {
        return normal() != Normal3d();
    }

    inline bool isLight() const {
        return type == VertexType::Light ||  (type == VertexType::Surface && si()->primitive()->light());
    }

    inline bool isDeltaLight() const {
        return false;
    }

    inline bool isIBL() const {
        //TODO: In the future, followling line should be revised for directional light.
        return type == VertexType::Light &&
               (!ei()->light || ei()->light->type() == LightType::Envmap);
    }

    // Public static inline methods

    static inline Vertex createCamera(const Camera* camera, const Ray& ray,
                                      const Spectrum& beta) {
        return Vertex(VertexType::Camera, EndpointInteraction(camera, ray), beta);
    }

    static inline Vertex createCamera(const Camera* camera, const Interaction& it,
                                      const Spectrum& beta) {
        return Vertex(VertexType::Camera, EndpointInteraction(it, camera), beta);
    }

    static inline Vertex createLight(const EndpointInteraction& ei,
                                     const Spectrum& beta, double pdf) {
        Vertex v(VertexType::Light, ei, beta);
        v.pdfFwd = pdf;
        return v;
    }

    static inline Vertex createLight(const Light& light, const Ray& ray,
                                     const Normal3d& nrmLight, const Spectrum& Le,
                                     double pdf) {
        Vertex v(VertexType::Light, EndpointInteraction(&light, ray, nrmLight), Le);
        v.pdfFwd = pdf;
        return v;
    }

    static inline Vertex createSurface(const SurfaceInteraction& isect,
                                       const Spectrum& beta, double pdf,
                                       const Vertex& prev) {
        Vertex v(isect, beta);
        v.pdfFwd = prev.convertDensity(pdf, v);
        return v;
    }

    static inline Vertex createMedium(const MediumInteraction& mi,
                                      const Spectrum& beta, double pdf,
                                      const Vertex& prev) {
        Vertex v(mi, beta);
        v.pdfFwd = prev.convertDensity(pdf, v);
        return v;
    }

    // Public fields
    VertexType type;
    Spectrum beta;
    std::variant<EndpointInteraction, MediumInteraction, SurfaceInteraction> intr;
    double pdfFwd = 0.0;
    double pdfRev = 0.0;
    bool delta = false;
};


int randomWalk(const Scene& scene, Ray ray, Sampler& sampler,
               MemoryArena& arena, Spectrum beta, double pdf, int maxDepth,
               Vertex* path, bool isCamera);

int calcCameraSubpath(const Scene& scene, Sampler& sampler,
                      MemoryArena& arena, int maxDepth,
                      const Camera& camera, const Point2i& pixel,
                      const Point2d& randFilm,
                      Vertex* path);

int calcLightSubpath(const Scene& scene, Sampler& sampler,
                     MemoryArena& arena, int maxDepth, const Distribution1D& lightDist,
                     Vertex* path);

double calcMISWeight(const Scene& scene,
                     Vertex* lightPath, Vertex* cameraPath,
                     Vertex& sampled,
                     int lightID, int cameraID,
                     const Distribution1D& lightDist);

Spectrum G(const Scene& scene, Sampler& sampler, const Vertex& v0,
           const Vertex& v1);

Spectrum connectBDPT(const Scene& scene,
                     Vertex* lightPath, Vertex* cameraPath,
                     int lightID, int cameraID, const Distribution1D& lightDist,
                     const Camera& camera, Sampler& sampler, Point2d* pRaster,
                     double* misWeight = nullptr);

}  // namespace spica

#endif  // _SPICA_BIDIRECTIONAL_H_
