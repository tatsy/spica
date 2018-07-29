#include "bidirectional.h"

#include "core/camera.h"
#include "core/scene.h"
#include "core/sampling.h"
#include "core/bsdf.h"
#include "core/phase.h"
#include "core/visibility_tester.h"

namespace spica {

double densityIBL(const Scene& scene, const Distribution1D& lightDist,
                  const Vector3d& w) {
    double pdf = 0.0;
    const int nLights = (int)scene.lights().size();
    for (int i = 0; i < nLights; i++) {
        const auto& light = scene.lights()[i];
        if (light->type() == LightType::Envmap) {
            pdf += light->pdfLi(Interaction(), -w) * lightDist(i);
        }
    }
    return pdf / (lightDist.integral() * lightDist.count());
}

Vertex::Vertex()
    : intr{ } {
}

Vertex::Vertex(VertexType type_, const EndpointInteraction& ei, const Spectrum& beta_)
    : type{ type_ }
    , beta{ beta_ }
    , intr{ }{
    intr.emplace<0>(ei);
}

Vertex::Vertex(const SurfaceInteraction& si, const Spectrum& beta_)
    : type{ VertexType::Surface }
    , beta{ beta_ }
    , intr{ } {
    intr.emplace<2>(si);
}

Vertex::Vertex(const MediumInteraction& mi, const Spectrum& beta_)
    : type{ VertexType::Medium }
    , beta{ beta_ }
    , intr{ } {
    intr.emplace<1>(mi);
}

Vertex::Vertex(const Vertex& v) {
    operator=(v);
}

Vertex& Vertex::operator=(const Vertex& v) {
    this->type = v.type;
    this->beta = v.beta;
    this->intr = v.intr;
    this->pdfFwd = v.pdfFwd;
    this->pdfRev = v.pdfRev;
    this->pdfRev = v.pdfRev;
    this->delta = v.delta;
    return *this;
}

Spectrum Vertex::Le(const Scene& scene, const Vertex& v) const {
    if (!isLight()) return Spectrum(0.0);

    Vector3d w = v.pos() - this->pos();
    if (w.squaredNorm() == 0.0) return Spectrum(0.0);

    w = w.normalized();
    if (isIBL()) {
        Spectrum ret(0.0);
        for (const auto& l : scene.lights()) {
            ret += l->Le(Ray(pos(), -w));
        }
        return ret;
    } else {
        const Light *l = si()->primitive()->light();
        Assertion(l != nullptr && l->isArea(), "Area light not detected");
        return l->L(*si(), w);
    }
}

Spectrum Vertex::f(const Vertex& next) const {
    Vector3d wi = next.pos() - this->pos();
    if (wi.squaredNorm() == 0.0) return Spectrum(0.0);

    wi = wi.normalized();
    switch (type) {
        case VertexType::Surface:
            return si()->bsdf()->f(si()->wo(), wi);

        case VertexType::Medium:
            return Spectrum(mi()->phase()->p(mi()->wo(), wi));

        default:
            FatalError("Vertex::f() not implemented!!");
            return Spectrum(0.0);
    }
}

bool Vertex::isConnectible() const {
    switch (type) {
        case VertexType::Medium:
            return true;

        case VertexType::Light:
            // In the future, followling line should be revised for directional light.
            return true;

        case VertexType::Camera:
            return true;

        case VertexType::Surface:
            return si()->bsdf()->numComponents(BxDFType::Diffuse | BxDFType::Glossy |
                                            BxDFType::Reflection |
                                            BxDFType::Transmission) > 0;
    }

    FatalError("Unhandled vertex type in isConnectible()");
    return false;
}

double Vertex::convertDensity(double pdf, const Vertex& next) const {
    // Convert PDF to that considers solid angle density.
    if (next.isIBL()) return pdf;

    Vector3d w = next.pos() - this->pos();
    double dist2 = w.squaredNorm();
    if (dist2 == 0.0) return 0.0;

    double invDist2 = 1.0 / dist2;
    if (next.isOnSurface()) {
        pdf *= vect::absDot(next.normal(), w * std::sqrt(invDist2));
    }
    return pdf * invDist2;
}

double Vertex::pdf(const Scene& scene, const Vertex* prev,
           const Vertex& next) const {
    if (type == VertexType::Light) return pdfLight(scene, next);

    Vector3d wn = next.pos() - this->pos();
    if (wn.squaredNorm() == 0.0) return 0.0;

    wn = wn.normalized();
    Vector3d wp;
    if (prev) {
        wp = prev->pos() - this->pos();
        if (wp.squaredNorm() == 0.0) return 0.0;
        wp = wp.normalized();
    } else {
        Assertion(type == VertexType::Camera, "Here, type should be camera");
    }

    double pdf = 0.0, unused;
    if (type == VertexType::Camera) {
        ei()->camera->pdfWe(ei()->spawnRay(wn), &unused, &pdf);
    } else if (type == VertexType::Surface) {
        pdf = si()->bsdf()->pdf(wp, wn);
    } else if (type == VertexType::Medium) {
        pdf = mi()->phase()->p(wp, wn);
    } else {
        FatalError("Vertex::pdf() not implemented");
    }

    return convertDensity(pdf, next);
}

double Vertex::pdfLight(const Scene& scene, const Vertex& v) const {
    Vector3d w = v.pos() - this->pos();
    double invDist2 = 1.0 / w.squaredNorm();
    w *= std::sqrt(invDist2);

    double pdf;
    if (isIBL()) {
        Bounds3d b = scene.worldBound();
        Point3d worldCenter = (b.posMin() + b.posMax()) * 0.5;
        double worldRadius = (b.posMax() - worldCenter).norm();
        pdf = 1.0 / (PI * worldRadius * worldRadius);
    } else {
        Assertion(isLight(), "Here, vertex type should be light");
        const Light* light = type == VertexType::Light ? ei()->light
                                                       : si()->primitive()->light();
        Assertion(light != nullptr, "Light is null");

        double pdfPos, pdfDir;
        light->pdfLe(Ray(pos(), w), normal(), &pdfPos, &pdfDir);
        pdf = pdfDir * invDist2;
    }

    if (v.isOnSurface()) pdf *= vect::absDot(v.normal(), w);
    return pdf;
}

double Vertex::pdfLightOrigin(const Scene& scene, const Vertex& v,
                      const Distribution1D& lightDist) const {
    Vector3d w = v.pos() - this->pos();
    if (w.squaredNorm() == 0.0) return 0.0;

    w = w.normalized();
    if (isIBL()) {
        return densityIBL(scene, lightDist, w);
    } else {
        double pdfPos, pdfDir, pdfChoise = 0.0;
        Assertion(isLight(), "This object should not be light.");

        const Light* light = type == VertexType::Light ? ei()->light
                                                       : si()->primitive()->light();
        Assertion(light != nullptr, "Light is nullptr");

        const int nLights = static_cast<int>(scene.lights().size());
        for (int i = 0; i < nLights; i++) {
            if (scene.lights()[i].get() == light) {
                pdfChoise = lightDist.pdfDiscrete(i);
                break;
            }
        }
        Assertion(pdfChoise != 0.0, "Current light is not included in the scene");

        light->pdfLe(Ray(pos(), w), normal(), &pdfPos, &pdfDir);
        return pdfPos * pdfChoise;
    }
}

int randomWalk(const Scene& scene, Ray ray, Sampler& sampler,
               MemoryArena& arena, Spectrum beta, double pdf, int maxDepth,
               Vertex* path, bool isCamera) {
    if (maxDepth == 0) return 0;

    int bounces = 0;
    double pdfFwd = pdf, pdfRev = 0.0;
    while (true) {
        MediumInteraction mi;

        SurfaceInteraction isect;
        bool isIntersect = scene.intersect(ray, &isect);

        if (ray.medium()) {
            beta *= ray.medium()->sample(ray, sampler, arena, &mi);
        }

        if (beta.isBlack()) break;

        Vertex& vertex = path[bounces];
        Vertex& prev   = path[bounces - 1];
        if (mi.isValid()) {
            // Process medium interaction.
            vertex = Vertex::createMedium(mi, beta, pdfFwd, prev);
            if (++bounces >= maxDepth) break;

            Vector3d wi;
            pdfFwd = pdfRev = mi.phase()->sample(-ray.dir(), &wi, sampler.get2D());
            ray = mi.spawnRay(wi);
        } else {
            // Process surface interaction.
            if (!isIntersect) {
                if (isCamera) {
                    vertex = Vertex::createLight(EndpointInteraction(ray), beta, pdfFwd);
                    ++bounces;
                }
                break;
            }

            // If medium boundary is detected, current intersection is skipped.
            isect.setScatterFuncs(ray, arena);
            if (!isect.bsdf()) {
                ray = isect.spawnRay(ray.dir());
                continue;
            }

            // Store surface interaction.
            vertex = Vertex::createSurface(isect, beta, pdfFwd, prev);
            if (++bounces >= maxDepth) break;

            // Sample next direction and compute reverse probability.
            Vector3d wi, wo = isect.wo();
            BxDFType type;
            Spectrum f = isect.bsdf()->sample(wo, &wi, sampler.get2D(), &pdfFwd,
                                              BxDFType::All, &type);
            if (f.isBlack() || pdfFwd == 0.0) break;

            beta *= f * vect::absDot(wi, isect.ns()) / pdfFwd;
            pdfRev = isect.bsdf()->pdf(wi, wo, BxDFType::All);
            if ((type & BxDFType::Specular) != BxDFType::None) {
                vertex.delta = true;
                pdfRev = pdfFwd = 0.0;
            }
            ray = isect.spawnRay(wi);
        }
        prev.pdfRev = vertex.convertDensity(pdfRev, prev);
    }
    return bounces;
}

int calcCameraSubpath(const Scene& scene, Sampler& sampler,
                      MemoryArena& arena, int maxDepth,
                      const Camera& camera, const Point2i& pixel,
                      const Point2d& randFilm,
                      Vertex* path) {
    if (maxDepth == 0) return 0;

    Point2d randLens = sampler.get2D();

    Ray ray = camera.spawnRay(pixel, randFilm, randLens);
    Spectrum beta(1.0);

    path[0] = Vertex::createCamera(&camera, ray, beta);

    double pdfPos, pdfDir;
    camera.pdfWe(ray, &pdfPos, &pdfDir);
    return randomWalk(scene, ray, sampler, arena, beta, pdfDir, maxDepth - 1, path + 1, true) + 1;
}

int calcLightSubpath(const Scene& scene, Sampler& sampler,
                     MemoryArena& arena, int maxDepth, const Distribution1D& lightDist,
                     Vertex* path) {
    if (maxDepth == 0) return 0;

    // Sample light.
    double lightPdf;
    const int lightID = lightDist.sampleDiscrete(sampler.get1D(), &lightPdf);
    const auto& light = scene.lights()[lightID];

    // Generate a ray, and compute contributing light radiance.
    Ray ray;
    Normal3d nrmLight;
    double pdfPos, pdfDir;
    Spectrum Le = light->sampleLe(sampler.get2D(), sampler.get2D(), &ray,
                                  &nrmLight, &pdfPos, &pdfDir);
    if (pdfPos == 0.0 || pdfDir == 0.0 || Le.isBlack()) return 0;

    // Create light end point vertex.
    path[0] = Vertex::createLight(*light, ray, nrmLight, Le, pdfPos * lightPdf);
    Spectrum beta = Le * vect::absDot(nrmLight, ray.dir()) / (lightPdf * pdfPos * pdfDir);

    // Compute a path by random walk.
    int bounces = randomWalk(scene, ray, sampler, arena, beta, pdfDir,
                             maxDepth - 1, path + 1, false);

    // Correct sampling density for image-based light.
    if (path[0].isIBL()) {
        if (bounces > 0) {
            path[1].pdfFwd = pdfPos;
            if (path[1].isOnSurface()) {
                path[1].pdfFwd *= vect::absDot(ray.dir(), path[1].normal());
            }
        }

        path[0].pdfFwd = densityIBL(scene, lightDist, ray.dir());
    }

    return bounces + 1;
}

double calcMISWeight(const Scene& scene,
                     Vertex* lightPath, Vertex* cameraPath,
                     Vertex& sampled,
                     int lightID, int cameraID,
                     const Distribution1D& lightDist) {
    // Single bounce connection.
    if (lightID + cameraID == 2) return 1.0;

    // To handle specular reflectio, zero contribution is remapped to 1.0.
    auto remap0 = [](double f) -> double { return f != 0.0 ? f : 1.0; };

    // Take current and previous sample.
    Vertex* vl = lightID > 0 ? &lightPath[lightID - 1] : nullptr;
    Vertex* vc = cameraID > 0 ? &cameraPath[cameraID - 1] : nullptr;
    Vertex* vlMinus = lightID > 1 ? &lightPath[lightID - 2] : nullptr;
    Vertex* vcMinus = cameraID > 1 ? &cameraPath[cameraID - 2] : nullptr;

    // Temporal swaps in this function scope.
    ScopedAssignment<Vertex> a1;
    if (lightID == 1) {
        a1 = { vl, sampled };
    } else if (cameraID == 1) {
        a1 = { vc, sampled };
    }

    ScopedAssignment<bool> a2, a3;
    if (vc) a2 = { &vc->delta, false };
    if (vl) a3 = { &vl->delta, false };

    ScopedAssignment<double> a4;
    if (vc) {
        a4 = { &vc->pdfRev, lightID > 0
                            ? vl->pdf(scene, vlMinus, *vc)
                            : vc->pdfLightOrigin(scene, *vcMinus, lightDist) };
    }

    ScopedAssignment<double> a5;
    if (vcMinus) {
        a5 = { &vcMinus->pdfRev, lightID > 0 ? vc->pdf(scene, vl, *vcMinus)
                                             : vc->pdfLight(scene, *vcMinus) };
    }

    ScopedAssignment<double> a6;
    if (vl) {
        a6 = { &vl->pdfRev, vc->pdf(scene, vcMinus, *vl) };
    }
    ScopedAssignment<double> a7;
    if (vlMinus) {
        a7 = { &vlMinus->pdfRev, vl->pdf(scene, vc, *vlMinus) };
    }

    // Compute sum of path contributions of the same length.
    double sumRi = 0.0;
    double ri = 1.0;
    for (int i = cameraID - 1; i > 0; i--) {
        ri *= remap0(cameraPath[i].pdfRev) / remap0(cameraPath[i].pdfFwd);
        if (cameraPath[i].delta || cameraPath[i - 1].delta) continue;

        sumRi += ri;
    }

    ri = 1.0;
    for (int i = lightID - 1; i >= 0; i--) {
        ri *= remap0(lightPath[i].pdfRev) / remap0(lightPath[i].pdfFwd);
        bool isDeltaLight = i > 0 ? lightPath[i - 1].delta
                                  : lightPath[0].isDeltaLight();
        if (lightPath[i].delta || isDeltaLight) continue;

        sumRi += ri;
    }

    return 1.0 / (1.0 + sumRi);
}

Spectrum G(const Scene& scene, Sampler& sampler, const Vertex& v0,
           const Vertex& v1) {
    Vector3d d = v0.pos() - v1.pos();
    double g = 1.0 / d.squaredNorm();

    d *= std::sqrt(g);
    if (v0.isOnSurface()) g *= vect::absDot(v0.normal(), d);
    if (v1.isOnSurface()) g *= vect::absDot(v1.normal(), d);

    VisibilityTester vis(v0.getInteraction(), v1.getInteraction());
    return g * vis.transmittance(scene, sampler);
}

Spectrum connectBDPT(const Scene& scene,
                     Vertex* lightPath, Vertex* cameraPath,
                     int lightID, int cameraID, const Distribution1D& lightDist,
                     const Camera& camera, Sampler& sampler, Point2d* pRaster,
                     double* misWeight) {
    Spectrum L(0.0);
    if (cameraID > 1 && lightID != 0 &&
        cameraPath[cameraID - 1].type == VertexType::Light) {
        return Spectrum(0.0);
    }

    Vertex sampled;
    if (lightID == 0) {
        const Vertex& vc = cameraPath[cameraID - 1];
        if (vc.isLight()) L = vc.Le(scene, cameraPath[cameraID - 2]) * vc.beta;
    } else if (cameraID == 1) {
        const Vertex& vl = lightPath[lightID - 1];
        if (vl.isConnectible()) {
            VisibilityTester vis;
            Vector3d wi;
            double pdf;
            Spectrum Wi = camera.sampleWi(vl.getInteraction(), sampler.get2D(),
                                          &wi, &pdf, pRaster, &vis);
            if (pdf > 0.0 && !Wi.isBlack()) {
                sampled = Vertex::createCamera(&camera, vis.p2(), Wi / pdf);
                L = vl.beta * vl.f(sampled) * sampled.beta;
                if (vl.isOnSurface()) L *= vect::absDot(wi, vl.normal());
                if (!L.isBlack()) L *= vis.transmittance(scene, sampler);
            }
        }
    } else if (lightID == 1) {
        const Vertex& vc = cameraPath[cameraID - 1];
        if (vc.isConnectible()) {
            double lightPdf;
            VisibilityTester vis;
            Vector3d wi;
            double pdf;
            int id = lightDist.sampleDiscrete(sampler.get1D(), &lightPdf);
            const auto& l = scene.lights()[id];

            Spectrum lightWeight = l->sampleLi(vc.getInteraction(), sampler.get2D(),
                                               &wi, &pdf, &vis);

            if (pdf > 0.0 && !lightWeight.isBlack()) {
                EndpointInteraction ei(vis.p2(), l.get());
                sampled = Vertex::createLight(ei, lightWeight / (pdf * lightPdf), 0.0);
                sampled.pdfFwd = sampled.pdfLightOrigin(scene, vc, lightDist);
                L = vc.beta * vc.f(sampled) * sampled.beta;

                if (vc.isOnSurface()) L *= vect::absDot(wi, vc.normal());
                if (!L.isBlack()) L *= vis.transmittance(scene, sampler);
            }
        }
    } else {
        const Vertex& vc = cameraPath[cameraID - 1];
        const Vertex& vl = lightPath[lightID - 1];
        if (vc.isConnectible() && vl.isConnectible()) {
            L = vc.beta * vc.f(vl) * vl.f(vc) * vl.beta;
            if (!L.isBlack()) L *= G(scene, sampler, vl, vc);
        }
    }

    double misW = L.isBlack() ? 0.0 : calcMISWeight(scene, lightPath, cameraPath,
                                                    sampled, lightID, cameraID,
                                                    lightDist);
    Assertion(!std::isnan(misW), "Invalid MIS weight!!");

    L *= misW;
    if (misWeight) *misWeight = misW;

    return L;
}

}  // namespace spica