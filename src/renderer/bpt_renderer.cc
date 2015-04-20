#define SPICA_BPT_RENDERER_EXPORT
#include "bpt_renderer.h"

#include <cstdio>
#include <vector>
#include <algorithm>

#include "Camera.h"

namespace spica {

    namespace {

        // --------------------------------------------------
        // Structs for storing path/light tracing results
        // --------------------------------------------------

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


        /* Struct for storing traced vertices
         */
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


        // --------------------------------------------------
        // Sample functions
        // --------------------------------------------------

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

        // --------------------------------------------------
        // Specular reflectance model
        // --------------------------------------------------
        Vector3 reflectionVec(const Vector3& v, const Vector3& n) {
            return (v - n * 2.0 * n.dot(v)).normalize();
        }

        bool isTotalReflection(const bool isIncoming, const Vector3& position, const Vector3& in, const Vector3& normal, const Vector3& orientNormal,
                               Vector3& reflectDir, Vector3& refractDir, double& fresnelRef, double& fresnelTransmit) {
            reflectDir = reflectionVec(in, normal);

            // Snell's rule
            const double nnt = isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM;
            const double ddn = in.dot(orientNormal);
            const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

            if (cos2t < 0.0) {  // Total reflection
                refractDir = Vector3();
                fresnelRef = 1.0;
                fresnelTransmit = 0.0;
                return true;
            }

            refractDir = (in * nnt - normal * (isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))).normalize();

            const double a = IOR_OBJECT - IOR_VACCUM;
            const double b = IOR_OBJECT + IOR_VACCUM;
            const double R0 = (a * a) / (b * b);

            const double c = 1.0 - (isIncoming ? -ddn : refractDir.dot(-orientNormal));
            fresnelRef = R0 + (1.0 - R0) * pow(c, 5.0);
            fresnelTransmit = 1.0 - fresnelRef;

            return false;
        }

        // --------------------------------------------------
        // Multiple importance sampling
        // --------------------------------------------------

        double calcPdfA(const Scene& scene, const Camera& camera, const std::vector<const Vertex*>& verts, const int prevFromIdx, const int fromIdx, const int nextIdx) {
            static const double reflectProb = 0.5;

            const Vertex& fromVert = *verts[fromIdx];
            const Vertex& nextVert = *verts[nextIdx];
            Vertex const* prevFromVertex = NULL;
            if (0 <= prevFromIdx && prevFromIdx < verts.size()) {
                prevFromVertex = verts[prevFromIdx];
            }

            const Vector3 to = nextVert.position - fromVert.position;
            const Vector3 normalizedTo = to.normalize();
            double pdf = 0.0;

            if (fromVert.objtype == Vertex::OBJECT_TYPE_LIGHT || fromVert.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                pdf = sample_hemisphere_pdf_omega(fromVert.orientNormal, normalizedTo);
            } else if (fromVert.objtype == Vertex::OBJECT_TYPE_LENS) {
                const Ray testRay(nextVert.position, -normalizedTo);
                Vector3 positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                const double lensT = camera.intersectLens(testRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                if (EPS < lensT) {
                    const Vector3 x0xI = positionOnSensor - positionOnLens;
                    const Vector3 x0xV = positionOnObjplane - positionOnLens;
                    const Vector3 x0x1 = testRay.origin() - positionOnLens;

                    const double PImage = 1.0 / (camera.sensor().width() * camera.sensor().height());
                    const double PAx1 = camera.PImageToPAx1(PImage, x0xV, x0x1, nextVert.orientNormal);
                    return PAx1;
                }
                return 0.0;
            } else if (fromVert.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                const Primitive* tempObj = scene.getObjectPtr(fromVert.objectId);
                if (tempObj->reftype() == REFLECTION_SPECULAR) {
                    if (prevFromVertex != NULL) {
                        const Vector3 intoFromVertexDir = (fromVert.position - prevFromVertex->position).normalize();
                        const bool isIncoming = intoFromVertexDir.dot(fromVert.objectNormal) < 0.0;
                        const Vector3 fromNewOrientNormal = isIncoming ? fromVert.objectNormal : -fromVert.objectNormal;

                        Vector3 reflectDir;
                        Vector3 refractDir;
                        double fresnelRef;
                        double fresnelTransmit;
                        if (isTotalReflection(isIncoming, fromVert.position, intoFromVertexDir, fromVert.objectNormal, fromNewOrientNormal,
                                              reflectDir, refractDir, fresnelRef, fresnelTransmit)) {
                            pdf = 1.0;
                        } else {
                            pdf = fromNewOrientNormal.dot(normalizedTo) > 0.0 ? reflectProb : 1.0 - reflectProb;
                        }
                    }
                } else {
                    pdf = 1.0;
                }
            }

            const Vector3 nextNewOrientNormal = to.dot(nextVert.objectNormal) < 0.0 ? nextVert.objectNormal : -nextVert.objectNormal;
            return pdf * (-1.0 * normalizedTo).dot(nextNewOrientNormal) / to.dot(to);
         }

        double calcMISWeight(const Scene& scene, const Camera& camera, const double totalPdfA, const std::vector<Vertex>& eyeVerts, const std::vector<Vertex>& lightVerts, const int nEyeVerts, const int nLightVerts) {
            const Primitive* tempObj;
            double rouletteProb;

            std::vector<const Vertex*> verts(nEyeVerts + nLightVerts);
            std::vector<double> pi1pi(nEyeVerts + nLightVerts);
            const Sphere* lightSphere = reinterpret_cast<const Sphere*>(scene.getObjectPtr(scene.lightId()));
            const double PAy0 = sample_sphere_pdf_A(lightSphere->radius());
            const double PAx0 = camera.samplingPdfOnLens();

            const int k = nEyeVerts + nLightVerts - 1;
            for (int i = 0; i < nLightVerts; i++) {
                verts[i] = &lightVerts[i];
            }
            for (int i = nEyeVerts - 1; i >= 0; i--) {
                verts[nLightVerts + nEyeVerts - i - 1] = &eyeVerts[i]; 
            }

            // Russian roulette probability
            tempObj = scene.getObjectPtr(verts[0]->objectId);
            rouletteProb = tempObj->emission().norm() > 1.0 ? 1.0 : std::max(tempObj->color().x(), std::max(tempObj->color().y(), tempObj->color().z()));
            pi1pi[0] = PAy0 / (calcPdfA(scene, camera, verts, 2, 1, 0) * rouletteProb);
            for (int i = 1; i < k; i++) {
                const double a = calcPdfA(scene, camera, verts, i - 2, i - 1, i);
                const double b = calcPdfA(scene, camera, verts, i + 2, i + 1, i);
                pi1pi[i] = a / b;
            }

            // tempObj = scene.getObjectPtr(verts[k]->objectId);
            // rouletteProb = tempObj->emission().norm() > 1.0 ? 1.0 : std::max(tempObj->color().x(), std::max(tempObj->color().y(), tempObj->color().z()));
            pi1pi[k] = 0.0; //(calcPdfA(scene, camera, verts, k - 2, k - 1, k) * rouletteProb) / PAx0;

            // require p
            std::vector<double> p(nEyeVerts + nLightVerts + 1);
            p[nLightVerts] = totalPdfA;
            for (int i = nLightVerts; i <= k; i++) {
                p[i + 1] = p[i] * pi1pi[i];
            }
            for (int i = nLightVerts - 1; i >= 0; i--) {
                p[i] = p[i + 1] / pi1pi[i];
            }

            // Specular
            for (int i = 0; i < verts.size(); i++) {
                if (verts[i]->objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                    p[i] = 0.0;
                    p[i + 1] = 0.0;
                }
            }

            // Power-heuristic
            double MIS = 0.0;
            for (int i = 0; i < p.size(); i++) {
                const double v = p[i] / p[nLightVerts];
                MIS += v * v;
            }
            return 1.0 / MIS;
        }

        // --------------------------------------------------
        // Light and path tracer
        // --------------------------------------------------

        LightTracingResult executeLightTracing(const Scene& scene, const Camera& camera, const Random& rng, std::vector<Vertex>& vertices) {
            // Generate sample on the light
            double pdfAreaOnLight;
            const int lightId = scene.lightId();
            const Sphere* lightSphere = reinterpret_cast<const Sphere*>(scene.getObjectPtr(lightId));

            const Vector3 positionOnLight = lightSphere->center() + sample_sphere(lightSphere->radius(), rng, pdfAreaOnLight);
            const Vector3 normalOnLight = (positionOnLight - lightSphere->center()).normalize();

            double totalPdfA = pdfAreaOnLight;

            vertices.push_back(Vertex(positionOnLight, normalOnLight, normalOnLight, lightId, Vertex::OBJECT_TYPE_LIGHT, totalPdfA, Color(0, 0, 0)));

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
                    totalPdfA *= nowSamplePdfArea;

                    // Geometry term
                    const double G = x0x1.normalize().dot(camera.sensor().direction().normalize()) * (-1.0) * (x0x1.normalize().dot(prevNormal) / x0x1.dot(x0x1));
                    throughputMC *= G;
                    vertices.push_back(Vertex(positionOnLens, camera.sensor().direction().normalize(), camera.sensor().direction().normalize(), -1, Vertex::OBJECT_TYPE_LENS, totalPdfA, throughputMC));
                
                    const Color result = (camera.contribSensitivity(x0xV, x0xI, x0x1) * throughputMC) / totalPdfA;
                    return LightTracingResult(result, x, y, true);
                }

                if (!isHitScene) {
                    break;
                }

                const Primitive* currentObj = scene.getObjectPtr(intersection.objectId());
                const HitPoint& hitpoint = intersection.hitPoint();

                const Vector3 orientNormal = hitpoint.normal().dot(nowRay.direction()) < 0.0 ? hitpoint.normal() : -1.0 * hitpoint.normal();
                const double rouletteProb = currentObj->emission().norm() > 1.0 ? 1.0 : std::max(currentObj->color().x(), std::max(currentObj->color().y(), currentObj->color().z()));
                
                if (rng.randReal() >= rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3 toNextVertex = nowRay.origin() - hitpoint.position();
                const double nowSampledPdfArea = nowSampledPdfOmega * (toNextVertex.normalize().dot(orientNormal) / toNextVertex.dot(toNextVertex));
                totalPdfA *= nowSampledPdfArea;

                const double G = toNextVertex.normalize().dot(orientNormal) * (-1.0 * toNextVertex).normalize().dot(prevNormal) / toNextVertex.dot(toNextVertex);
                throughputMC = G * throughputMC;

                vertices.push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId(),
                                          currentObj->reftype() == REFLECTION_DIFFUSE ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                          totalPdfA, throughputMC));

                if (currentObj->reftype() == REFLECTION_DIFFUSE) {
                    const Vector3 nextDir = sample_hemisphere_cos_term(orientNormal, rng, nowSampledPdfOmega);
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / PI;

                } else if (currentObj->reftype() == REFLECTION_SPECULAR) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3 nextDir = Vector3::reflect(nowRay.direction(), hitpoint.normal());
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));
                } else if (currentObj->reftype() == REFLECTION_REFRACTION) {
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    Vector3 reflectDir;
                    Vector3 refractDir;
                    double fresnelRef;
                    double fresnelTransmit;
                    if (!isTotalReflection(isIncoming, hitpoint.position(), nowRay.direction(), hitpoint.normal(), orientNormal,
                        reflectDir, refractDir, fresnelRef, fresnelTransmit)) {

                        if (rng.randReal() < REFLECT_PROBABLITY) {
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), reflectDir);
                            throughputMC = fresnelRef * (currentObj->color().cwiseMultiply(throughputMC)) / (toNextVertex.normalize().dot(orientNormal));
                            totalPdfA *= REFLECT_PROBABLITY;
                        } else {
                            const double nnt2 = pow(isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM, 2.0);
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), refractDir);
                            throughputMC = nnt2 * fresnelTransmit * (currentObj->color().cwiseMultiply(throughputMC)) / (toNextVertex.normalize().dot(orientNormal));
                            totalPdfA *= (1.0 - REFLECT_PROBABLITY);
                        }
                    } else { // Total reflection
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(hitpoint.position(), reflectDir);
                        throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));
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
                const double rouletteProb = currentObj->emission().norm() > 1.0 ? 1.0 : std::max(currentObj->color().x(), std::max(currentObj->color().y(), currentObj->color().z()));

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
                    totalPdfA *= PAx1;

                    throughputMC = camera.contribSensitivity(x0xV, x0xI, x0x1) * throughputMC;
                } else {
                    const double nowSampledPdfA = nowSampledPdfOmega * (toNextVertex.normalize().dot(orientNormal)) / toNextVertex.dot(toNextVertex);
                    totalPdfA *= nowSampledPdfA;
                }

                // Geometry term
                const double G = toNextVertex.normalize().dot(orientNormal) * (-1.0 * toNextVertex).normalize().dot(prevNormal) / toNextVertex.dot(toNextVertex);
                throughputMC = G * throughputMC;

                if (currentObj->emission().norm() > 0.0) {
                    vertices.push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId(), Vertex::OBJECT_TYPE_LIGHT, totalPdfA, throughputMC));
                    const Color result = throughputMC.cwiseMultiply(currentObj->emission()) / totalPdfA;
                    return PathTracingResult(result, x, y, true);
                }

                vertices.push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId(),
                                          currentObj->reftype() == REFLECTION_DIFFUSE ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                          totalPdfA, throughputMC));

                if (currentObj->reftype() == REFLECTION_DIFFUSE) {
                    const Vector3 nextDir = sample_hemisphere_cos_term(orientNormal, rng, nowSampledPdfOmega);
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / PI;

                } else if (currentObj->reftype() == REFLECTION_SPECULAR) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3 nextDir = Vector3::reflect(nowRay.direction(), hitpoint.normal());
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));

                } else if (currentObj->reftype() == REFLECTION_REFRACTION) {
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    Vector3 reflectDir;
                    Vector3 refractDir;
                    double fresnelRef;
                    double fresnelTransmit;
                    if (!isTotalReflection(isIncoming, hitpoint.position(), nowRay.direction(), hitpoint.normal(), orientNormal,
                                           reflectDir, refractDir, fresnelRef, fresnelTransmit)) {
                    
                        if (rng.randReal() < REFLECT_PROBABLITY) {
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), reflectDir);
                            throughputMC = fresnelRef * (currentObj->color().cwiseMultiply(throughputMC)) / (toNextVertex.normalize().dot(orientNormal));
                            totalPdfA *= REFLECT_PROBABLITY;
                        } else {
                            const double nnt2 = pow(isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM, 2.0);
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), refractDir);
                            throughputMC = nnt2 * fresnelTransmit * (currentObj->color().cwiseMultiply(throughputMC)) / (toNextVertex.normalize().dot(orientNormal));
                            totalPdfA *= (1.0 - REFLECT_PROBABLITY);
                        }
                    } else { // Total reflection
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(hitpoint.position(), reflectDir);
                        throughputMC = currentObj->color().cwiseMultiply(throughputMC) / (toNextVertex.normalize().dot(orientNormal));                    
                    }
                }
                prevNormal = orientNormal;
            }

            return PathTracingResult(Color(), 0, 0, false);
        }

        struct Sample {
            int imageX;
            int imageY;
            Color value;
            bool startFromPixel;

            Sample(const int imageX_, const int imageY_, const Color& value_, const bool startFromPixel_)
                : imageX(imageX_)
                , imageY(imageY_)
                , value(value_)
                , startFromPixel(startFromPixel_)
            {
            }
        };

        struct BPTResult {
            std::vector<Sample> samples;
        };

        BPTResult executeBPT(const Scene& scene, const Camera& camera, const Random& rng, int x, int y) {
            BPTResult bptResult;

            std::vector<Vertex> eyeVerts, lightVerts;
            const PathTracingResult ptResult = executePathTracing(scene, camera, x, y, rng, eyeVerts);
            const LightTracingResult ltResult = executeLightTracing(scene, camera, rng, lightVerts);

            if (ptResult.isLightHit) {
                const double weightMIS = calcMISWeight(scene, camera, eyeVerts[eyeVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, (const int)eyeVerts.size(), 0);
                const Color result = weightMIS * ptResult.value;
                bptResult.samples.push_back(Sample(x, y, result, true));
            }

            if (ltResult.isLensHit) {
                const double weightMIS = calcMISWeight(scene, camera, lightVerts[lightVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, 0, (const int)lightVerts.size());
                const int lx = ltResult.imageX;
                const int ly = ltResult.imageY;
                const Color result = weightMIS * ltResult.value;
                bptResult.samples.push_back(Sample(lx, ly, result, false));
            }

            for (int eyeVertId = 1; eyeVertId <= eyeVerts.size(); eyeVertId++) {
                for (int lightVertId = 1; lightVertId <= lightVerts.size(); lightVertId++) {
                    int targetX = x;
                    int targetY = y;
                    const Vertex& eyeEnd = eyeVerts[eyeVertId - 1];
                    const Vertex& lightEnd = lightVerts[lightVertId - 1];

                    const double totalPdfA = eyeEnd.totalPdfA * lightEnd.totalPdfA;
                    if (totalPdfA == 0.0) {
                        continue;
                    }

                    Color eyeThoughput = eyeEnd.throughput;
                    Color lightThrouput = lightEnd.throughput;
                    Color connectedThrought = Color(1.0, 1.0, 1.0);

                    if (lightVertId == 1) {
                        lightThrouput = scene.getObjectPtr(lightVerts[0].objectId)->emission();
                    }

                    // End-to-end ray tracing
                    Intersection intersection;
                    const Vector3 lendToEend = eyeEnd.position - lightEnd.position;
                    const Ray testRay(lightEnd.position, lendToEend.normalize());
                    scene.intersect(testRay, intersection);

                    if (eyeEnd.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                        const Primitive* eyeEndObj = scene.getObjectPtr(eyeEnd.objectId);
                        connectedThrought = connectedThrought.cwiseMultiply(eyeEndObj->color()) / PI;
                        double dist = (intersection.hitPoint().position() - eyeEnd.position).norm();
                        if ((intersection.hitPoint().position() - eyeEnd.position).norm() >= EPS) {
                            continue;
                        }
                    } else if (eyeEnd.objtype == Vertex::OBJECT_TYPE_LENS) {
                        Vector3 positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                        const double lensT = camera.intersectLens(testRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                        if (EPS < lensT && lensT < intersection.hitPoint().distance()) {
                            const Vector3 x0xI = positionOnSensor - positionOnLens;
                            const Vector3 x0xV = positionOnObjplane - positionOnLens;
                            const Vector3 x0x1 = testRay.origin() - positionOnLens;

                            targetX = (int)uvOnSensor.x();
                            targetY = (int)uvOnSensor.y();
                            targetX = targetX < 0 ? 0 : targetX >= camera.imageWidth() ? camera.imageWidth() - 1 : targetX;
                            targetY = targetY < 0 ? 0 : targetY >= camera.imageHeight() ? camera.imageHeight() - 1 : targetY;

                            connectedThrought = camera.contribSensitivity(x0xV, x0xI, x0x1) * connectedThrought;
                        } else {
                            continue;
                        }
                    } else if (eyeEnd.objtype == Vertex::OBJECT_TYPE_LIGHT || eyeEnd.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                        continue;
                    }

                    if (lightEnd.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                        const Primitive* tempObj = scene.getObjectPtr(lightEnd.objectId);
                        connectedThrought = connectedThrought.cwiseMultiply(tempObj->color()) / PI;
                    } else if (lightEnd.objtype == Vertex::OBJECT_TYPE_LIGHT) {

                    } else if (lightEnd.objtype == Vertex::OBJECT_TYPE_LENS || lightEnd.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                        continue;
                    }

                    double G = std::max(0.0, (-1.0 * lendToEend.normalize().dot(eyeEnd.orientNormal)));
                    G *= std::max(0.0, lendToEend.normalize().dot(lightEnd.orientNormal));
                    G /= lendToEend.dot(lendToEend);
                    connectedThrought = G * connectedThrought;

                    const double weightMIS = calcMISWeight(scene, camera, totalPdfA, eyeVerts, lightVerts, eyeVertId, lightVertId);
                    if (isnan(weightMIS)) {
                        continue;
                    }

                    const Color result = weightMIS * connectedThrought.cwiseMultiply(eyeThoughput).cwiseMultiply(lightThrouput) / totalPdfA;
                    bptResult.samples.push_back(Sample(targetX, targetY, result, eyeVertId > 1.0));
                }
            }

            return bptResult;
        }

        bool isInvalidValue(const Color& color) {
            if (isnan(color.x()) || isnan(color.y()) || isnan(color.z())) return true;
            if (color.x() < 0.0 || INFTY < color.x()) return true;
            if (color.y() < 0.0 || INFTY < color.y()) return true;
            if (color.z() < 0.0 || INFTY < color.z()) return true;
            return false;
        }

        bool isValidValue(const Color& color) {
            return !isInvalidValue(color);
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

    int BPTRenderer::render(const Scene& scene, const Camera& camera) {
        Image image(_width, _height);

        for (int it = 0; it < _samplePerPixel; it++) {
            for (int y = 0; y < _height; y++) {
                printf("%d - %d\n", it, y);
                for (int x = 0; x < _width; x++) {
                    BPTResult bptResult = executeBPT(scene, camera, rng, x, y);
                    
                    for (int i = 0; i < bptResult.samples.size(); i++) {
                        const int ix = bptResult.samples[i].imageX;
                        const int iy = bptResult.samples[i].imageY;
                        if (isValidValue(bptResult.samples[i].value)) {
                            if (bptResult.samples[i].startFromPixel) {
                                image.pixel(ix, iy) += bptResult.samples[i].value;     
                            } else {
                                image.pixel(ix, iy) += bptResult.samples[i].value / ((double)_width * _height);
                            }
                        }

                    }
                }
            }
        }

        Image output(_width, _height);
        for (int y = 0; y < _height; y++) {
            for (int x = 0; x < _width; x++) {
                output.pixel(x, y) = image.pixel(_width - x - 1, y) / _samplePerPixel;
            }
        }

        output.savePPM("simplebpt.ppm");
        return 0;
    }

    int BPTRenderer::renderPT(const Scene& scene, const Camera& camera) {
        const int width = camera.imageWidth();
        const int height = camera.imageHeight();
        const int spp = _samplePerPixel;

        Image image(width, height);
        for (int sample = 0; sample < spp; sample++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    std::vector<Vertex> vertices;
                    const PathTracingResult result = executePathTracing(scene, camera, x, y, rng, vertices);

                    if (result.isLightHit) {
                        if (isValidValue(result.value)) {
                            image.pixel(x, y) += result.value;
                        }
                    }
                }
            }
        }

        Image output(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                output.pixel(x, y) = image.pixel(width - x - 1, y) / spp;
            }
        }

        output.savePPM("bpt_pt_part.ppm");
        return 0;
    }

    int BPTRenderer::renderLT(const Scene& scene, const Camera& camera) {
        const int width = camera.imageWidth();
        const int height = camera.imageHeight();
        const int spp = _samplePerPixel * width * height;

        Image image(width, height);

        for (int sample = 0; sample < spp; sample++) {
            std::vector<Vertex> vertices;
            LightTracingResult result = executeLightTracing(scene, camera, rng, vertices);

            if (result.isLensHit) {
                if (isValidValue(result.value)) {
                    int x = result.imageX;
                    int y = result.imageY;
                    image.pixel(x, y) += result.value;
                }
            }
        }

        Image output(width, height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                output.pixel(x, y) = image.pixel(width - x - 1, y) / spp;
                const Color& color = output(x, y);
            }
        }

        output.savePPM("bpt_lt_part.ppm");
        return 0;
    }
}
