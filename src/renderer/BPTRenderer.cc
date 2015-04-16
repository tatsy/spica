#define SPICA_BPT_RENDERER_EXPORT
#include "BPTRenderer.h"

#include <algorithm>

namespace spica {

    namespace {
        double sample_hemisphere_pdf_omega(const Vector3& normal, const Vector3& direction) {
            return std::max(normal.dot(direction), 0.0) / PI;
        }

        Vector3 sample_hemisphere_cos_term(const Vector3& normal, const Random& rng, double& pdf_omega) {
            Vector3 w, u, v;
            w = normal;
            if (abs(w.x()) > EPS) {
                u = Vector3(0.0, 1.0, 0.0).cross(w).normalize();
            } else {
                u = Vector3(1.0, 0.0, 0.0).cross(w).normalize();
            }
            v = w.cross(u);

            const double r1 = 2.0 * PI * rng.randReal();
            const double r2 = rng.randReal();
            const double r2s = sqrt(r2);

            Vector3 nextDir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)).normalize();

            pdf_omega = sample_hemisphere_pdf_omega(normal, nextDir);

            return nextDir;
        }

        double sample_sphere_pdf_A(const double R) {
            return 1.0 / (4.0 * PI * R * R);
        }

        Vector3 sample_sphere(const double R, const Random& rng, double& pdfA) {
            const double z = rng.randReal() * 2.0 - 1.0;
            const double sz = sqrt(1.0 - z * z);
            const double phi = 2.0 * PI * rng.randReal();

            pdfA = sample_sphere_pdf_A(R);

            return R * Vector3(sz * cos(phi), sz * sin(phi), z);
        }
    }
    
    BPTRenderer::BPTRenderer(int width, int height, int samples, int supsamples)
        : RendererBase(width, height, samples, supsamples)
    {
    }

    BPTRenderer::BPTRenderer(const BPTRenderer& renderer)
        : RendererBase(renderer)
    {
    }

    BPTRenderer::~BPTRenderer()
    {
    }

    BPTRenderer& BPTRenderer::operator=(const BPTRenderer& renderer) {
        RendererBase::operator=(renderer);
        return *this;
    }

    int BPTRenderer::render(const Scene& scene) {
        for (int i = 0; i < _samplePerPixel; i++) {
            for (int y = 0; y < _height; y++) {
                for (int x = 0; x < _width; x++) {
                    
                }
            }
        }
        return 0;
    }

    void BPTRenderer::executeBPT(const Scene& scene, int x, int y) {

    }

    void BPTRenderer::executeLightTracing(const Scene& scene, int x, int y) {
        // Generate sample on the light
        double pdfOnLight = 1.0;

        const Sphere* lightSphere = reinterpret_cast<const Sphere*>(scene.getObjectPtr(scene.lightId()));

        double randZ = rng.randReal();
        double randSZ = sqrt(1.0 - randZ * randZ);
        double randPhi = rng.randReal() * 2.0 * PI;

        const Vector3 positionOnLight = lightSphere->center() + sample_sphere(lightSphere->radius(), rng, pdfOnLight);
        const Vector3 normalOnLight = (positionOnLight - lightSphere->center()).normalize();
        double totalPdf = pdfOnLight;

        Color throughputMC = lightSphere->emission();

        double nowSampledPdfOmega;
        const Vector3 nextDir = sample_hemisphere_cos_term(normalOnLight, rng, nowSampledPdfOmega);

        Ray nowRay(positionOnLight, nextDir);
        Vector3 prevNormal = normalOnLight;

        for (;;) {
            Intersection intersection;
            const bool isHitScene = scene.intersect(nowRay, intersection);

            // Check if ray hits lens

        }
    }

    void BPTRenderer::executePathTracing(const Scene& scene, int x, int y) {
    }
}
