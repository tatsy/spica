#define SPICA_BPT_RENDERER_EXPORT
#include "bpt_renderer.h"

#include <cstdio>
#include <vector>
#include <algorithm>

#include "renderer_helper.h"
#include "../utils/common.h"
#include "../utils/sampler.h"

namespace spica {

    namespace {

        // --------------------------------------------------
        // Structs for storing path/light tracing results
        // --------------------------------------------------
        enum HitObjectType {
            HIT_ON_LIGHT,
            HIT_ON_LENS,
            HIT_ON_OBJECT
        };

        struct TraceResult {
            Color value;
            int imageX;
            int imageY;
            HitObjectType hitObjType;

            TraceResult(const Color& value_, const int imageX_, const int imageY_, HitObjectType hitObjType_)
                : value(value_)
                , imageX(imageX_)
                , imageY(imageY_)
                , hitObjType(hitObjType_)
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

        double sample_sphere_pdf_A(const double R) {
            return 1.0 / (4.0 * PI * R * R);
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
            const Vector3 normalizedTo = to.normalized();
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

                    const double PImage = 1.0 / (camera.sensorW() * camera.sensorH());
                    const double PAx1 = camera.PImageToPAx1(PImage, x0xV, x0x1, nextVert.orientNormal);
                    return PAx1;
                }
                return 0.0;
            } else if (fromVert.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                const Material& mtrl = scene.getMaterial(fromVert.objectId);
                if (mtrl.reftype == REFLECTION_SPECULAR) {
                    if (prevFromVertex != NULL) {
                        const Vector3 intoFromVertexDir = (fromVert.position - prevFromVertex->position).normalized();
                        const bool isIncoming = intoFromVertexDir.dot(fromVert.objectNormal) < 0.0;
                        const Vector3 fromNewOrientNormal = isIncoming ? fromVert.objectNormal : -fromVert.objectNormal;

                        Vector3 reflectDir;
                        Vector3 refractDir;
                        double fresnelRef;
                        double fresnelTransmit;
                        
                        if (helper::isTotalRef(isIncoming, fromVert.position, intoFromVertexDir, fromVert.objectNormal, fromNewOrientNormal,
                                              &reflectDir, &refractDir, &fresnelRef, &fresnelTransmit)) {
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
            std::vector<const Vertex*> verts(nEyeVerts + nLightVerts);
            std::vector<double> pi1pi(nEyeVerts + nLightVerts);
            const Primitive* light = scene.get(scene.lightID());
            const double PAy0 = 1.0 / light->area();
            const double PAx0 = 1.0 / camera.lensArea();

            const int k = nEyeVerts + nLightVerts - 1;
            for (int i = 0; i < nLightVerts; i++) {
                verts[i] = &lightVerts[i];
            }
            for (int i = nEyeVerts - 1; i >= 0; i--) {
                verts[nLightVerts + nEyeVerts - i - 1] = &eyeVerts[i]; 
            }

            // Russian roulette probability
            const Primitive* curobj = scene.get(verts[0]->objectId);
            const Material& mtrl = scene.getMaterial(verts[0]->objectId);
            double roulette = mtrl.emission.norm() > 1.0 ? 1.0 : std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));
            pi1pi[0] = PAy0 / (calcPdfA(scene, camera, verts, 2, 1, 0) * roulette);
            for (int i = 1; i < k; i++) {
                const double a = calcPdfA(scene, camera, verts, i - 2, i - 1, i);
                const double b = calcPdfA(scene, camera, verts, i + 2, i + 1, i);
                pi1pi[i] = a / b;
            }

            pi1pi[k] = 0.0;

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

        TraceResult executeLightTracing(const Scene& scene, const Camera& camera, RandomSeq& rseq, std::vector<Vertex>* vertices, const int bounceLimit) {
            // Generate sample on the light
            const int lightId = scene.lightID();
            const Primitive* light = scene.get(lightId);

            Vector3 positionOnLight, normalOnLight;
            const double r1Light = rseq.next();
            const double r2Light = rseq.next();
            sampler::on(light, &positionOnLight, &normalOnLight, r1Light, r2Light);
            double pdfAreaOnLight = 1.0 / light->area();

            double totalPdfA = pdfAreaOnLight;

            vertices->push_back(Vertex(positionOnLight, normalOnLight, normalOnLight, lightId, Vertex::OBJECT_TYPE_LIGHT, totalPdfA, Color(0, 0, 0)));

            Color throughputMC = scene.getMaterial(lightId).emission;

            Vector3 nextDir;
            const double r1 = rseq.next();
            const double r2 = rseq.next();
            sampler::onHemisphere(normalOnLight, &nextDir, r1, r2);
            double nowSampledPdfOmega = sample_hemisphere_pdf_omega(normalOnLight, nextDir);

            Ray nowRay(positionOnLight, nextDir);
            Vector3 prevNormal = normalOnLight;

            for (int bounce = 1; bounce <= bounceLimit; bounce++) {
                // Get next random
                std::vector<double> randnums;
                rseq.next(3, &randnums);

                Intersection intersection;
                const bool isHitScene = scene.intersect(nowRay, intersection);

                Vector3 positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                double lensT = camera.intersectLens(nowRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                if (EPS < lensT && lensT < intersection.hitpoint().distance()) {
                    const Vector3 x0xI = positionOnSensor - positionOnLens;
                    const Vector3 x0xV = positionOnObjplane - positionOnLens;
                    const Vector3 x0x1 = nowRay.origin() - positionOnLens;

                    int x = (int)uvOnSensor.x();
                    int y = (int)uvOnSensor.y();
                    x = x < 0 ? 0 : x >= camera.imageW() ? camera.imageW() - 1 : x;
                    y = y < 0 ? 0 : y >= camera.imageH() ? camera.imageH() - 1 : y;

                    const double nowSamplePdfArea = nowSampledPdfOmega * (x0x1.normalized().dot(camera.direction().normalized()) / x0x1.dot(x0x1));
                    totalPdfA *= nowSamplePdfArea;

                    // Geometry term
                    const double G = x0x1.normalized().dot(camera.direction().normalized()) * (-1.0) * (x0x1.normalized().dot(prevNormal) / x0x1.dot(x0x1));
                    throughputMC *= G;
                    vertices->push_back(Vertex(positionOnLens, camera.direction().normalized(), camera.direction().normalized(), -1, Vertex::OBJECT_TYPE_LENS, totalPdfA, throughputMC));
                
                    const Color result = Color((camera.contribSensitivity(x0xV, x0xI, x0x1) * throughputMC) / totalPdfA);
                    return TraceResult(result, x, y, HIT_ON_LENS);
                }

                if (!isHitScene) {
                    break;
                }

                const Material& mtrl = scene.getMaterial(intersection.objectId());
                const Hitpoint& hitpoint = intersection.hitpoint();

                const Vector3 orientNormal = hitpoint.normal().dot(nowRay.direction()) < 0.0 ? hitpoint.normal() : -1.0 * hitpoint.normal();
                const double rouletteProb = mtrl.emission.norm() > 1.0 ? 1.0 : std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));
                
                if (randnums[0] >= rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3 toNextVertex = nowRay.origin() - hitpoint.position();
                const double nowSampledPdfArea = nowSampledPdfOmega * (toNextVertex.normalized().dot(orientNormal) / toNextVertex.dot(toNextVertex));
                totalPdfA *= nowSampledPdfArea;

                const double G = toNextVertex.normalized().dot(orientNormal) * (-1.0 * toNextVertex).normalized().dot(prevNormal) / toNextVertex.dot(toNextVertex);
                throughputMC = G * throughputMC;

                vertices->push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId(),
                                          mtrl.reftype == REFLECTION_DIFFUSE ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                          totalPdfA, throughputMC));

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    sampler::onHemisphere(orientNormal, &nextDir, randnums[1], randnums[2]);
                    nowSampledPdfOmega = sample_hemisphere_pdf_omega(orientNormal, nextDir);
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = mtrl.color.multiply(throughputMC) / PI;
                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3 nextDir = Vector3::reflect(nowRay.direction(), hitpoint.normal());
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = mtrl.color.multiply(throughputMC) / (toNextVertex.normalized().dot(orientNormal));
                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    Vector3 reflectDir;
                    Vector3 refractDir;
                    double fresnelRef;
                    double fresnelTransmit;
                    if (!helper::isTotalRef(isIncoming, hitpoint.position(), nowRay.direction(), hitpoint.normal(), orientNormal,
                        &reflectDir, &refractDir, &fresnelRef, &fresnelTransmit)) {

                        if (randnums[1] < REFLECT_PROBABLITY) {
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), reflectDir);
                            throughputMC = fresnelRef * (mtrl.color.multiply(throughputMC)) / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= REFLECT_PROBABLITY;
                        } else {
                            const double nnt2 = pow(isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM, 2.0);
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), refractDir);
                            throughputMC = nnt2 * fresnelTransmit * (mtrl.color.multiply(throughputMC)) / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= (1.0 - REFLECT_PROBABLITY);
                        }
                    } else { // Total reflection
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(hitpoint.position(), reflectDir);
                        throughputMC = mtrl.color.multiply(throughputMC) / (toNextVertex.normalized().dot(orientNormal));
                    }

                }
                prevNormal = orientNormal;
            }

            return TraceResult(Color(), 0, 0, HIT_ON_OBJECT);
        }

        TraceResult executePathTracing(const Scene& scene, const Camera& camera, int x, int y, RandomSeq& rseq, std::vector<Vertex>* vertices, const int bounceLimit) {
            // Sample point on lens and object plane
            CameraSample camSample = camera.sample(x, y, rseq);

            double totalPdfA = camSample.pdfLens;

            Color throughputMC = Color(1.0, 1.0, 1.0);

            vertices->push_back(Vertex(camSample.posLens, camera.lensNormal(), camera.lensNormal(), -1, Vertex::OBJECT_TYPE_LENS, totalPdfA, throughputMC));
        
            Ray nowRay = camSample.generateRay();
            double nowSampledPdfOmega = 1.0;
            Vector3 prevNormal = camera.lensNormal();

            for (int bounce = 1; bounce <= bounceLimit; bounce++) {
                // Get next random
                std::vector<double> randnums;
                rseq.next(3, &randnums);

                Intersection intersection;
                if (!scene.intersect(nowRay, intersection)) {
                    break;
                }

                const Material& mtrl = scene.getMaterial(intersection.objectId());
                const Hitpoint& hitpoint = intersection.hitpoint();

                const Vector3 orientNormal = hitpoint.normal().dot(nowRay.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();
                const double rouletteProb = mtrl.emission.norm() > 1.0 ? 1.0 : std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));

                if (randnums[0] > rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3 toNextVertex = nowRay.origin() - hitpoint.position();
                if (bounce == 1) {
                    const Vector3 x0xI = camSample.posSensor - camSample.posLens;
                    const Vector3 x0xV = camSample.posObjectPlane - camSample.posLens;
                    const Vector3 x0x1 = hitpoint.position() - camSample.posLens;
                    const double PAx1 = camera.PImageToPAx1(camSample.pdfImage, x0xV, x0x1, orientNormal);
                    totalPdfA *= PAx1;

                    throughputMC = camera.contribSensitivity(x0xV, x0xI, x0x1) * throughputMC;
                } else {
                    const double nowSampledPdfA = nowSampledPdfOmega * (toNextVertex.normalized().dot(orientNormal)) / toNextVertex.squaredNorm();
                    totalPdfA *= nowSampledPdfA;
                }

                // Geometry term
                const double G = toNextVertex.normalized().dot(orientNormal) * (-1.0 * toNextVertex).normalized().dot(prevNormal) / toNextVertex.squaredNorm();
                throughputMC = G * throughputMC;

                if (mtrl.emission.norm() > 0.0) {
                    vertices->push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId(), Vertex::OBJECT_TYPE_LIGHT, totalPdfA, throughputMC));
                    const Color result = Color(throughputMC.multiply(mtrl.emission) / totalPdfA);
                    return TraceResult(result, x, y, HIT_ON_LIGHT);
                }

                vertices->push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), intersection.objectId(),
                                           mtrl.reftype == REFLECTION_DIFFUSE ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                           totalPdfA, throughputMC));

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    Vector3 nextDir;
                    sampler::onHemisphere(orientNormal, &nextDir, randnums[1], randnums[2]);
                    nowSampledPdfOmega = sample_hemisphere_pdf_omega(orientNormal, nextDir);
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = mtrl.color.multiply(throughputMC) / PI;

                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3 nextDir = Vector3::reflect(nowRay.direction(), hitpoint.normal());
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughputMC = mtrl.color.multiply(throughputMC) / (toNextVertex.normalized().dot(orientNormal));

                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    Vector3 reflectDir;
                    Vector3 refractDir;
                    double fresnelRef;
                    double fresnelTransmit;
                    if (!helper::isTotalRef(isIncoming, hitpoint.position(), nowRay.direction(), hitpoint.normal(), orientNormal,
                                           &reflectDir, &refractDir, &fresnelRef, &fresnelTransmit)) {
                    
                        if (randnums[1] < REFLECT_PROBABLITY) {
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), reflectDir);
                            throughputMC = fresnelRef * (mtrl.color.multiply(throughputMC)) / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= REFLECT_PROBABLITY;
                        } else {
                            const double nnt2 = pow(isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM, 2.0);
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), refractDir);
                            throughputMC = nnt2 * fresnelTransmit * (mtrl.color.multiply(throughputMC)) / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= (1.0 - REFLECT_PROBABLITY);
                        }
                    } else {
                        // Total reflection
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(hitpoint.position(), reflectDir);
                        throughputMC = mtrl.color.multiply(throughputMC) / (toNextVertex.normalized().dot(orientNormal));                    
                    }
                }
                prevNormal = orientNormal;
            }

            return TraceResult(Color(), 0, 0, HIT_ON_OBJECT);
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

        BPTResult executeBPT(const Scene& scene, const Camera& camera, RandomSeq& rseq, int x, int y, const int bounceLimit) {
            BPTResult bptResult;

            std::vector<Vertex> eyeVerts, lightVerts;
            const TraceResult ptResult = executePathTracing(scene, camera, x, y, rseq, &eyeVerts, bounceLimit);
            const TraceResult ltResult = executeLightTracing(scene, camera, rseq, &lightVerts, bounceLimit);

            if (ptResult.hitObjType == HIT_ON_LIGHT) {
                const double weightMIS = calcMISWeight(scene, camera, eyeVerts[eyeVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, (const int)eyeVerts.size(), 0);
                const Color result = Color(weightMIS * ptResult.value);
                bptResult.samples.push_back(Sample(x, y, result, true));
            }

            if (ltResult.hitObjType == HIT_ON_LENS) {
                const double weightMIS = calcMISWeight(scene, camera, lightVerts[lightVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, 0, (const int)lightVerts.size());
                const int lx = ltResult.imageX;
                const int ly = ltResult.imageY;
                const Color result = Color(weightMIS * ltResult.value);
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
                        lightThrouput = scene.getMaterial(lightVerts[0].objectId).emission;
                    }

                    // End-to-end ray tracing
                    Intersection intersection;
                    const Vector3 lendToEend = eyeEnd.position - lightEnd.position;
                    const Ray testRay(lightEnd.position, lendToEend.normalized());
                    scene.intersect(testRay, intersection);

                    if (eyeEnd.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                        const Material& eyeEndMtrl = scene.getMaterial(eyeEnd.objectId);
                        connectedThrought = connectedThrought.multiply(eyeEndMtrl.color) / PI;
                        double dist = (intersection.hitpoint().position() - eyeEnd.position).norm();
                        if ((intersection.hitpoint().position() - eyeEnd.position).norm() >= EPS) {
                            continue;
                        }
                    } else if (eyeEnd.objtype == Vertex::OBJECT_TYPE_LENS) {
                        Vector3 positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                        const double lensT = camera.intersectLens(testRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                        if (EPS < lensT && lensT < intersection.hitpoint().distance()) {
                            const Vector3 x0xI = positionOnSensor - positionOnLens;
                            const Vector3 x0xV = positionOnObjplane - positionOnLens;
                            const Vector3 x0x1 = testRay.origin() - positionOnLens;

                            targetX = (int)uvOnSensor.x();
                            targetY = (int)uvOnSensor.y();
                            targetX = targetX < 0 ? 0 : targetX >= camera.imageW() ? camera.imageW() - 1 : targetX;
                            targetY = targetY < 0 ? 0 : targetY >= camera.imageH() ? camera.imageH() - 1 : targetY;

                            connectedThrought = camera.contribSensitivity(x0xV, x0xI, x0x1) * connectedThrought;
                        } else {
                            continue;
                        }
                    } else if (eyeEnd.objtype == Vertex::OBJECT_TYPE_LIGHT || eyeEnd.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                        continue;
                    }

                    if (lightEnd.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                        const Material& tempMtrl = scene.getMaterial(lightEnd.objectId);
                        connectedThrought = connectedThrought.multiply(tempMtrl.color) / PI;
                    } else if (lightEnd.objtype == Vertex::OBJECT_TYPE_LIGHT) {

                    } else if (lightEnd.objtype == Vertex::OBJECT_TYPE_LENS || lightEnd.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                        continue;
                    }

                    double G = std::max(0.0, (-1.0 * lendToEend.normalized().dot(eyeEnd.orientNormal)));
                    G *= std::max(0.0, lendToEend.normalized().dot(lightEnd.orientNormal));
                    G /= lendToEend.dot(lendToEend);
                    connectedThrought = G * connectedThrought;

                    const double weightMIS = calcMISWeight(scene, camera, totalPdfA, eyeVerts, lightVerts, eyeVertId, lightVertId);
                    if (isnan(weightMIS)) {
                        continue;
                    }

                    const Color result = Color(weightMIS * connectedThrought.multiply(eyeThoughput).multiply(lightThrouput) / totalPdfA);
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
    
    BDPTRenderer::BDPTRenderer()
    {
    }

    BDPTRenderer::~BDPTRenderer()
    {
    }

    void BDPTRenderer::render(const Scene& scene, const Camera& camera, const int samplePerPixel, const RandomType randType) {
        const int width  = camera.imageW();
        const int height = camera.imageH();
        const int bounceLimit = 32;

        RandomBase** rand = new RandomBase*[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            switch (randType) {
            case PSEUDO_RANDOM_TWISTER:
                printf("Use pseudo random numbers (Twister)\n");
                rand[i] = new Random();
                break;

            case QUASI_MONTE_CARLO:
                printf("Use quasi random numbers (Halton)\n");
                rand[i] = new Halton(200, true, i);
                break;

            default:
                msg_assert(false, "Unknown random number generator type!!");
            }
        }
        
        Image* buffer = new Image[OMP_NUM_CORE];
        for (int i = 0; i < OMP_NUM_CORE; i++) {
            buffer[i] = Image(width, height);
        }

        const int taskPerThread = (samplePerPixel + OMP_NUM_CORE - 1) / OMP_NUM_CORE;
        for (int t = 0; t < taskPerThread; t++) {
            ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
                RandomSeq rseq;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        rand[threadID]->requestSamples(rseq, 200);
                        BPTResult bptResult = executeBPT(scene, camera, rseq, x, y, bounceLimit);
                    
                        for (int i = 0; i < bptResult.samples.size(); i++) {
                            const int ix = bptResult.samples[i].imageX;
                            const int iy = bptResult.samples[i].imageY;
                            if (isValidValue(bptResult.samples[i].value)) {
                                omplock {
                                    if (bptResult.samples[i].startFromPixel) {
                                        buffer[threadID].pixel(ix, iy) += bptResult.samples[i].value;     
                                    } else {
                                        buffer[threadID].pixel(ix, iy) += bptResult.samples[i].value / ((double)width * height);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            char filename[256];
            Image image(width, height);
            const int usedSamples = (t + 1) * OMP_NUM_CORE;
            for (int k = 0; k < OMP_NUM_CORE; k++) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        image.pixel(width - x - 1, y) += buffer[k](x, y) / usedSamples;
                    }
                }
            }
            sprintf(filename, "bdpt_%03d.bmp", t + 1);
            image.saveBMP(filename);

            printf("  %6.2f %%  processed -> %s\r", 100.0 * (t + 1) / taskPerThread, filename);
        }
        printf("\nFinish!!\n");

        for (int i = 0; i < OMP_NUM_CORE; i++) {
            delete rand[i];
        }
        delete rand;
        delete[] buffer;
    }

}
