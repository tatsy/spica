#define SPICA_BPT_RENDERER_EXPORT
#include "BPTRenderer.h"

#include <vector>
#include <algorithm>

#include "Camera.h"

namespace spica {

    namespace {

        struct LightTracingResult {
            Color value;
            int imageX;
            int imageY;
            bool isLensHit;

            LightTracingResult(const Color& value_, const int imageX_, const int imageY_, const bool isLensHit_)
                : value(value_)
                , imageX(imageX_)
                , imageY(imageY_)
                , isLensHit(isLensHit_)
            {
            }
        };

        struct PathTracingResult {
            Color value;
            int imageX;
            int imageY;
            bool isLightHit;

            PathTracingResult(const Color& value_, const int imageX_, const int imageY_, const bool isLightHit_)
                : value(value_)
                , imageX(imageX_)
                , imageY(imageY_)
                , isLightHit(isLightHit_)
            {
            }
        };

        struct Vertex {
            double totalPdfA;
            Color throughput;
            Vector3 position;
            int objectId;
            Vector3 orientNormal;
            Vector3 objectNormal;

            enum ObjectType {
                OBJECT_TYPE_LIGHT,
                OBJECT_TYPE_LENS,
                OBJECT_TYPE_DIFFUSE,
                OBJECT_TYPE_SPECULAR,
            };

            ObjectType objtype;

            Vertex(const Vector3& position_, const Vector3 & orientNormal_, const Vector3& objectNormal_,
                   const int objectId_, const ObjectType objtype_, const double totalPdfA_, const Color& throughput_)
                : totalPdfA(totalPdfA_)
                , throughput(throughput_)
                , position(position_)
                , objectId(objectId_)
                , orientNormal(orientNormal_)
                , objectNormal(objectNormal_)
                , objtype(objtype_)
            {
            }
        };

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

        LightTracingResult executeLightTracing(const Scene& scene, const Camera& camera, const Random& rng, std::vector<Vertex>& vertices) {
            // Generate sample on the light
            double pdfAreaOnLight = 1.0;

            const int lightId = scene.lightId();
            const Sphere* lightSphere = reinterpret_cast<const Sphere*>(scene.getObjectPtr(lightId));

            const Vector3 positionOnLight = lightSphere->center() + sample_sphere(lightSphere->radius(), rng, pdfAreaOnLight);
            const Vector3 normalOnLight = (positionOnLight - lightSphere->center()).normalize();
            double totalPdfArea = pdfAreaOnLight;

            vertices.push_back(Vertex(positionOnLight, normalOnLight, normalOnLight, lightId, Vertex::OBJECT_TYPE_LIGHT, totalPdfArea, Color(0, 0, 0)));

            Color throughputMC = lightSphere->emission();

            double nowSampledPdfOmega;
            const Vector3 nextDir = sample_hemisphere_cos_term(normalOnLight, rng, nowSampledPdfOmega);

            Ray nowRay(positionOnLight, nextDir);
            Vector3 prevNormal = normalOnLight;

            for (;;) {
                Intersection intersection;
                const bool isHitScene = scene.intersect(nowRay, intersection);

                Vector3 positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                double lensT = camera.intersectLens(nowRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                if (EPS < lensT && lensT < intersection.hitPoint().distance()) {
                    const Vector3 x0xI = positionOnSensor - positionOnLens;
                    const Vector3 x0xV = positionOnObjplane - positionOnLens;
                    const Vector3 x0x1 = nowRay.origin() - positionOnLens;

                    int x = (int)uvOnSensor.x();
                    int y = (int)uvOnSensor.y();
                    x = x < 0 ? 0 : x >= camera.imageWidth() ? camera.imageWidth() - 1 : x;
                    y = y < 0 ? 0 : y >= camera.imageHeight() ? camera.imageHeight() - 1 : y;

                    const double nowSamplePdfArea = nowSampledPdfOmega * (x0x1.normalize().dot(camera.sensor().direction().normalize()) / x0x1.dot(x0x1));
                    totalPdfArea *= nowSamplePdfArea;

                    // Geometry term
                    const double G = x0x1.normalize().dot(camera.sensor().direction().normalize()) * (-1.0) * (x0x1.normalize().dot(prevNormal) / x0x1.dot(x0x1));

                    vertices.push_back(Vertex(positionOnLens, camera.sensor().direction().normalize(), camera.sensor().direction().normalize(), -1, Vertex::OBJECT_TYPE_LENS, totalPdfArea, throughputMC));
                
                    const Color result = (camera.contribSensitivity(x0xV, x0xI, x0x1) * throughputMC) / totalPdfArea;
                    return LightTracingResult(result, x, y, true);
                }

                if (!isHitScene) {
                    break;
                }

                const Primitive* currentObj = scene.getObjectPtr(intersection.objectId);
                const HitPoint& hitpoint = intersection.hitPoint();

                const Vector3 orientNormal = hitpoint.normal().dot(nowRay.direction()) < 0.0 ? hitpoint.normal() : -1.0 * hitpoint.normal();
                const double rouletteProb = currentObj->emission().norm() > 1.0 ? 1.0 : std::max(currentObj->emission().x(), std::max(currentObj->emission().y(), currentObj->emission().z()));
                
                if (rng.randReal() >= rouletteProb) {
                    break;
                }

                totalPdfArea *= rouletteProb;

                const Vector3 toNextVertex = nowRay.origin() - hitpoint.position();
                const double nowSampledPdfArea = nowSampledPdfOmega * (toNextVertex.normalize().dot(orientNormal) / toNextVertex.dot(toNextVertex));
                totalPdfArea *= rouletteProb;

                const double G = toNextVertex.normalize().dot(orientNormal) * (-1.0 * toNextVertex).normalize().dot(prevNormal) / toNextVertex.dot(toNextVertex);
                throughputMC = G * throughputMC;

                vertices.push_back(Vertex(hitpoint.position, orientNormal, hitpoint.normal(), intersection.objectId(),
                                          currentObj->reftype() == REFLECTION_DIFFUSE ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                          totalPdfArea, throughputMC));

                if (currentObj->reftype() == REFLECTION_DIFFUSE) {
                    const Vector3 nextDir = sample_hemisphere_cos_term(orientNormal, rng, nowSampledPdfOmega);
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / PI;

                } else if (currentObj->reftype() == REFLECTION_SPECULAR) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3 nextDir = nowRay.direction() - (2.0 * hitpoint.normal().dot(nowRay.direction())) * hitpoint.normal();
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));
                } else if (currentObj->reftype() == REFLECTION_REFRACTION) {
                    Vector3 reflectDir = nowRay.direction() - (2.0 * hitpoint.normal().dot(nowRay.direction())) * hitpoint.normal();
                    const Ray reflectRay = Ray(hitpoint.position(), reflectDir);

                    // Incoming or outgoing
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    // Snell's rule
                    const double nc = 1.0;
                    const double nt = indexOfRef;
                    const double nnt = isIncoming ? nc / nt : nt / nc;
                    const double ddn = nowRay.direction().dot(orientNormal);
                    const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

                    if (cos2t < 0.0) { // Total reflection
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(hitpoint.position(), reflectDir);
                        throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));
                    } else {
                        Vector3 refractDir = (nowRay.direction() * nnt - hitpoint.normal() * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalize();
                        const Ray refractRay = Ray(hitpoint.position(), refractDir);

                        // Schlick's approximation of Fresnel coefficient
                        const double a = nt - nc;
                        const double b = nt + nc;
                        const double R0 = (a * a) / (b * b);

                        const double c = 1.0 - (isIncoming ? -ddn : -refractDir.dot(orientNormal));
                        const double Re = R0 + (1.0 - R0) * pow(c, 5.0);
                        const double nnt2 = pow(isIncoming ? nc / nt : nt / nc, 2.0);
                        const double Tr = (1.0 - Re) * nnt2;

                        const double prob = 0.25 + 0.5 * Re;
                        if (rng.randReal() < prob) { // Reflect
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), reflectDir);
                            throughputMC = Re * currentObj->color().cwiseMultiply(throughputMC) / toNextVertex.normalize().dot(orientNormal);
                            totalPdfArea *= Re;
                        } else { // Refract
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), refractDir);
                            throughputMC = Tr * currentObj->color().cwiseMultiply(throughputMC) / toNextVertex.normalize().dot(orientNormal);
                        }
                    }
                }
                prevNormal = orientNormal;
            }

            return LightTracingResult(Color(), 0, 0, false);
        }

        PathTracingResult executePathTracing(const Scene& scene, const Camera& camera, int x, int y, const Random& rng, std::vector<Vertex>& vertices) {
            Vector3 positionOnSensor, positionOnObjplane, positionOnLens;

            double PImage, PLens;
            camera.samplePoints(x, y, rng, positionOnSensor, positionOnObjplane, positionOnLens, PImage, PLens);

            double totalPdfA = PLens;
            Color throughputMC = Color(1.0, 1.0, 1.0);

            vertices.push_back(Vertex(positionOnLens, camera.lens().normal(), camera.lens().normal(), -1, Vertex::OBJECT_TYPE_LENS, totalPdfA, throughputMC));
        
            Ray nowRay(positionOnLens, (positionOnObjplane - positionOnLens).normalize());
            double nowSampledPdfOmega = 1.0;
            Vector3 prevNormal = camera.lens().normal();

            for (int nowVertexIndex = 1; ; nowVertexIndex++) {
                Intersection intersection;
                if (!scene.intersect(nowRay, intersection)) {
                    break;
                }

                const Primitive* currentObj = scene.getObjectPtr(intersection.objectId());
                const HitPoint& hitpoint = intersection.hitPoint();

                const Vector3 orientNormal = hitpoint.normal().dot(nowRay.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();
                const double rouletteProb = currentObj->emission().norm() > 1.0 ? 1.0 : std::max(currentObj->emission().x(), std::max(currentObj->emission().y(), currentObj->emission().z()));

                if (rng.randReal() >= rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3 toNextVertex = nowRay.origin() - hitpoint.position();
                if (nowVertexIndex == 1) {
                    const Vector3 x0xI = positionOnSensor - positionOnLens;
                    const Vector3 x0xV = positionOnObjplane - positionOnLens;
                    const Vector3 x0x1 = hitpoint.position() - positionOnLens;
                    const double PAx1 = camera.PImageToPAx1(PImage, x0xV, x0x1, orientNormal);

                    throughputMC = camera.contribSensitivity(x0xV, x0xI, x0x1) * throughputMC;
                } else {
                    const double nowSampledPdfA = nowSampledPdfOmega * (toNextVertex.normalize().dot(orientNormal)) / toNextVertex.dot(toNextVertex);
                    totalPdfA *= nowSampledPdfA;
                }

                const double G = toNextVertex.normalize().dot(orientNormal) * (-1.0 * toNextVertex).normalize().dot(prevNormal) / toNextVertex.dot(toNextVertex);
                throughputMC = G * throughputMC;

                if (currentObj->emission().norm() > 0.0) {
                    vertices.push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId, Vertex::OBJECT_TYPE_LIGHT, totalPdfA, throughputMC));
                    const Color result = throughputMC.cwiseMultiply(currentObj->emission()) / totalPdfA;
                    return PathTracingResult(result, x, y, true);
                }

                vertices.push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId,
                                          currentObj->reftype() == REFLECTION_DIFFUSE ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                          totalPdfA, throughputMC));

                if (currentObj->reftype() == REFLECTION_DIFFUSE) {
                    const Vector3 nextDir = sample_hemisphere_cos_term(orientNormal, rng, nowSampledPdfOmega);
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / PI;

                } else if (currentObj->reftype() == REFLECTION_SPECULAR) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3 nextDir = nowRay.direction() - (2.0 * hitpoint.normal().dot(nowRay.direction())) * hitpoint.normal();
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));
                } else if (currentObj->reftype() == REFLECTION_REFRACTION) {
                    Vector3 reflectDir = nowRay.direction() - (2.0 * hitpoint.normal().dot(nowRay.direction())) * hitpoint.normal();
                    const Ray reflectRay = Ray(hitpoint.position(), reflectDir);

                    // Incoming or outgoing
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    // Snell's rule
                    const double nc = 1.0;
                    const double nt = indexOfRef;
                    const double nnt = isIncoming ? nc / nt : nt / nc;
                    const double ddn = nowRay.direction().dot(orientNormal);
                    const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

                    if (cos2t < 0.0) { // Total reflection
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(hitpoint.position(), reflectDir);
                        throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));
                    } else {
                        Vector3 refractDir = (nowRay.direction() * nnt - hitpoint.normal() * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalize();
                        const Ray refractRay = Ray(hitpoint.position(), refractDir);

                        // Schlick's approximation of Fresnel coefficient
                        const double a = nt - nc;
                        const double b = nt + nc;
                        const double R0 = (a * a) / (b * b);

                        const double c = 1.0 - (isIncoming ? -ddn : -refractDir.dot(orientNormal));
                        const double Re = R0 + (1.0 - R0) * pow(c, 5.0);
                        const double nnt2 = pow(isIncoming ? nc / nt : nt / nc, 2.0);
                        const double Tr = (1.0 - Re) * nnt2;

                        const double prob = 0.25 + 0.5 * Re;
                        if (rng.randReal() < prob) { // Reflect
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), reflectDir);
                            throughputMC = Re * currentObj->color().cwiseMultiply(throughputMC) / toNextVertex.normalize().dot(orientNormal);
                            totalPdfA *= Re;
                        } else { // Refract
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), refractDir);
                            throughputMC = Tr * currentObj->color().cwiseMultiply(throughputMC) / toNextVertex.normalize().dot(orientNormal);
                        }
                    }
                }
                prevNormal = orientNormal;
            }

            return PathTracingResult(Color(), 0, 0, false);
        }

    }  // unnamed namespace
    
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
}
