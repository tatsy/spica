#define SPICA_BPT_RENDERER_EXPORT
#include "bdpt.h"

#include <cstdio>
#include <vector>
#include <algorithm>

#include "renderer_helper.h"
#include "../utils/common.h"
#include "../utils/sampler.h"
#include "../random/random_sampler.h"

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
            Vector3D position;
            int objectId;
            Vector3D orientNormal;
            Vector3D objectNormal;

            enum ObjectType {
                OBJECT_TYPE_LIGHT,
                OBJECT_TYPE_LENS,
                OBJECT_TYPE_DIFFUSE,
                OBJECT_TYPE_SPECULAR,
            };

            ObjectType objtype;

            Vertex(const Vector3D& position_, const Vector3D & orientNormal_, const Vector3D& objectNormal_,
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

        double sample_hemisphere_pdf_omega(const Vector3D& normal, const Vector3D& direction) {
            return std::max(normal.dot(direction), 0.0) * INV_PI;
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

            const Vector3D to = nextVert.position - fromVert.position;
            const Vector3D normalizedTo = to.normalized();
            double pdf = 0.0;

            if (fromVert.objtype == Vertex::OBJECT_TYPE_LIGHT || fromVert.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                pdf = sample_hemisphere_pdf_omega(fromVert.orientNormal, normalizedTo);
            } else if (fromVert.objtype == Vertex::OBJECT_TYPE_LENS) {
                const Ray testRay(nextVert.position, -normalizedTo);
                Vector3D positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                const double lensT = camera.intersectLens(testRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                if (EPS < lensT) {
                    const Vector3D x0xI = positionOnSensor - positionOnLens;
                    const Vector3D x0xV = positionOnObjplane - positionOnLens;
                    const Vector3D x0x1 = testRay.origin() - positionOnLens;

                    const double PImage = 1.0 / (camera.sensorW() * camera.sensorH());
                    const double PAx1 = camera.PImageToPAx1(PImage, x0xV, x0x1, nextVert.orientNormal);
                    return PAx1;
                }
                return 0.0;
            } else if (fromVert.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                const BSDF& bsdf = scene.getBsdf(fromVert.objectId);
                if (bsdf.type() == BsdfType::Reflactive) {
                    if (prevFromVertex != NULL) {
                        const Vector3D intoFromVertexDir = (fromVert.position - prevFromVertex->position).normalized();
                        const bool isIncoming = intoFromVertexDir.dot(fromVert.objectNormal) < 0.0;
                        const Vector3D fromNewOrientNormal = isIncoming ? fromVert.objectNormal : -fromVert.objectNormal;

                        Vector3D reflectdir, transdir;
                        double fresnelRe, fresnelTr;
                        bool totalReflection = helper::checkTotalReflection(isIncoming,
                                                                            intoFromVertexDir, fromVert.objectNormal, fromNewOrientNormal,
                                                                             &reflectdir, &transdir, &fresnelRe, &fresnelTr);
                        if (totalReflection) {
                            pdf = 1.0;
                        } else {
                            pdf = fromNewOrientNormal.dot(normalizedTo) > 0.0 ? reflectProb : 1.0 - reflectProb;
                        }
                    }
                } else {
                    pdf = 1.0;
                }
            }

            const Vector3D nextNewOrientNormal = to.dot(nextVert.objectNormal) < 0.0 ? nextVert.objectNormal : -nextVert.objectNormal;
            return pdf * (-1.0 * normalizedTo).dot(nextNewOrientNormal) / to.dot(to);
        }

        double calcMISWeight(const Scene& scene, const Camera& camera, const double totalPdfA, const std::vector<Vertex>& eyeVerts, const std::vector<Vertex>& lightVerts, const int nEyeVerts, const int nLightVerts) {
            std::vector<const Vertex*> verts(nEyeVerts + nLightVerts);
            std::vector<double> pi1pi(nEyeVerts + nLightVerts);

            const double PAy0 = 1.0 / scene.totalLightArea();
            const double PAx0 = 1.0 / camera.lensArea();

            const int k = nEyeVerts + nLightVerts - 1;
            for (int i = 0; i < nLightVerts; i++) {
                verts[i] = &lightVerts[i];
            }
            for (int i = nEyeVerts - 1; i >= 0; i--) {
                verts[nLightVerts + nEyeVerts - i - 1] = &eyeVerts[i]; 
            }

            // Russian roulette probability
            const BSDF& bsdf = scene.getBsdf(verts[0]->objectId);
            const Color& emittance = scene.getEmittance(verts[0]->objectId);
            const Color& refl = bsdf.reflectance();
            double roulette = std::min(1.0, max3(refl.red(), refl.green(), refl.blue()));
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

        TraceResult lightTrace(const Scene& scene, const Camera& camera, Stack<double>& rstk, std::vector<Vertex>* vertices, const int bounceLimit) {
            // Generate sample on the light
            const int lightID     = scene.sampleLight(rstk.pop());
            const Triangle& light = scene.getTriangle(lightID);

            Vector3D lightPosition, lightNormal;
            sampler::onTriangle(light, &lightPosition, &lightNormal, rstk.pop(), rstk.pop());

            // Store vertex on light itself
            double totalPdfA = 1.0 / light.area();
            vertices->push_back(Vertex(lightPosition, lightNormal, lightNormal, lightID, Vertex::OBJECT_TYPE_LIGHT, totalPdfA, Color(0.0, 0.0, 0.0)));

            // Compute initial tracing direction
            Vector3D nextDir;
            sampler::onHemisphere(lightNormal, &nextDir, rstk.pop(), rstk.pop());
            double pdfOmega = sample_hemisphere_pdf_omega(lightNormal, nextDir);

            Ray currentRay(lightPosition, nextDir);
            Vector3D prevNormal = lightNormal;

            // Trace light ray
            Color throughput = scene.getEmittance(lightID);
            for (int bounce = 0; bounce < bounceLimit; bounce++) {
                const double rands[3] = { rstk.pop(), rstk.pop(), rstk.pop() };

                Intersection isect;
                const bool isHitScene = scene.intersect(currentRay, isect);

                // If ray hits on the lens, return curent result
                Vector3D positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                double lensT = camera.intersectLens(currentRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                if (EPS < lensT && lensT < isect.hitpoint().distance()) {
                    const Vector3D x0xI = positionOnSensor - positionOnLens;
                    const Vector3D x0xV = positionOnObjplane - positionOnLens;
                    const Vector3D x0x1 = currentRay.origin() - positionOnLens;

                    int x = (int)uvOnSensor.x();
                    int y = (int)uvOnSensor.y();
                    x = x < 0 ? 0 : x >= camera.imageW() ? camera.imageW() - 1 : x;
                    y = y < 0 ? 0 : y >= camera.imageH() ? camera.imageH() - 1 : y;

                    const double nowSamplePdfArea = pdfOmega * (x0x1.normalized().dot(camera.direction().normalized()) / x0x1.dot(x0x1));
                    totalPdfA *= nowSamplePdfArea;

                    // Geometry term
                    const double G = x0x1.normalized().dot(camera.direction().normalized()) * (-1.0) * (x0x1.normalized().dot(prevNormal) / x0x1.dot(x0x1));
                    throughput *= G;
                    vertices->push_back(Vertex(positionOnLens, camera.direction().normalized(), camera.direction().normalized(), -1, Vertex::OBJECT_TYPE_LENS, totalPdfA, throughput));
                
                    const Color result = Color((camera.contribSensitivity(x0xV, x0xI, x0x1) * throughput) / totalPdfA);
                    return TraceResult(result, x, y, HIT_ON_LENS);
                }

                if (!isHitScene) {
                    break;
                }

                // Otherwise, trace next direction
                const int triangleID = isect.objectId();
                const BSDF& bsdf = scene.getBsdf(triangleID);
                const Color& refl = bsdf.reflectance();
                const Color& emittance = scene.getEmittance(triangleID);
                const Hitpoint& hitpoint = isect.hitpoint();

                const Vector3D orientNormal = hitpoint.normal().dot(currentRay.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();
                const double rouletteProb = std::min(1.0, max3(refl.red(), refl.green(), refl.blue()));
                
                if (rands[0] >= rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3D toNextVertex = currentRay.origin() - hitpoint.position();
                const double nowSampledPdfArea = pdfOmega * (toNextVertex.normalized().dot(orientNormal) / toNextVertex.dot(toNextVertex));
                totalPdfA *= nowSampledPdfArea;

                const double G = toNextVertex.normalized().dot(orientNormal) * (-1.0 * toNextVertex).normalized().dot(prevNormal) / toNextVertex.dot(toNextVertex);
                throughput *= G;

                vertices->push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), isect.objectId(),
                                          bsdf.type() == BsdfType::Lambertian ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                          totalPdfA, throughput));

                if (bsdf.type() == BsdfType::Lambertian) {
                    sampler::onHemisphere(orientNormal, &nextDir, rands[1], rands[2]);
                    pdfOmega = sample_hemisphere_pdf_omega(orientNormal, nextDir);
                    currentRay = Ray(hitpoint.position(), nextDir);
                    throughput = bsdf.reflectance() * throughput * INV_PI;
                } else if (bsdf.type() == BsdfType::Specular) {
                    pdfOmega = 1.0;
                    const Vector3D nextDir = Vector3D::reflect(currentRay.direction(), hitpoint.normal());
                    currentRay = Ray(hitpoint.position(), nextDir);
                    throughput = bsdf.reflectance() * throughput / (toNextVertex.normalized().dot(orientNormal));
                } else if (bsdf.type() == BsdfType::Reflactive) {
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    Vector3D reflectdir, transdir;
                    double fresnelRe, fresnelTr;
                    bool totalReflection = helper::checkTotalReflection(isIncoming,
                                                                        currentRay.direction(), hitpoint.normal(), orientNormal,
                                                                        &reflectdir, &transdir, &fresnelRe, &fresnelTr);

                    if (totalReflection) {
                        pdfOmega = 1.0;
                        currentRay = Ray(hitpoint.position(), reflectdir);
                        throughput = bsdf.reflectance() * throughput / (toNextVertex.normalized().dot(orientNormal));
                    } else {
                        const double probability = 0.25 + 0.5 * kReflectProbability;
                        if (rands[1] < probability) {
                            pdfOmega = 1.0;
                            currentRay = Ray(hitpoint.position(), reflectdir);
                            throughput = fresnelRe * (bsdf.reflectance() * throughput) / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= probability;
                        } else {
                            const double ratio = isIncoming ? kIorVaccum / kIorObject : kIorObject / kIorVaccum;
                            const double nnt2   = ratio * ratio;
                            pdfOmega = 1.0;
                            currentRay = Ray(hitpoint.position(), transdir);
                            throughput = (nnt2 * fresnelTr) * (bsdf.reflectance() * throughput) / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= (1.0 - probability);
                        }
                    }
                }
                prevNormal = orientNormal;
            }

            return TraceResult(Color(), 0, 0, HIT_ON_OBJECT);
        }

        TraceResult pathTrace(const Scene& scene, const Camera& camera, int x, int y, Stack<double>& rstk, std::vector<Vertex>* vertices, const int bounceLimit) {
            // Sample point on lens and object plane
            CameraSample camSample = camera.sample(x, y, rstk);

            double totalPdfA = camSample.pdfLens;

            Color throughput = Color(1.0, 1.0, 1.0);

            vertices->push_back(Vertex(camSample.posLens, camera.lensNormal(), camera.lensNormal(), -1, Vertex::OBJECT_TYPE_LENS, totalPdfA, throughput));
        
            Ray nowRay = camSample.generateRay();
            double nowSampledPdfOmega = 1.0;
            Vector3D prevNormal = camera.lensNormal();

            for (int bounce = 0; bounce < bounceLimit; bounce++) {
                // Get next random
                const double rands[3] = { rstk.pop(), rstk.pop(), rstk.pop() };

                Intersection isect;
                if (!scene.intersect(nowRay, isect)) {
                    break;
                }

                const BSDF& bsdf = scene.getBsdf(isect.objectId());
                const Color& refl = bsdf.reflectance();
                const Color& emittance = scene.getEmittance(isect.objectId());
                const Hitpoint& hitpoint = isect.hitpoint();

                const Vector3D orientNormal = hitpoint.normal().dot(nowRay.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();
                const double rouletteProb = std::min(1.0, max3(refl.red(), refl.green(), refl.blue()));

                if (rands[0] > rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3D toNextVertex = nowRay.origin() - hitpoint.position();
                if (bounce == 0) {
                    const Vector3D x0xI = camSample.posSensor - camSample.posLens;
                    const Vector3D x0xV = camSample.posObjectPlane - camSample.posLens;
                    const Vector3D x0x1 = hitpoint.position() - camSample.posLens;
                    const double PAx1 = camera.PImageToPAx1(camSample.pdfImage, x0xV, x0x1, orientNormal);
                    totalPdfA *= PAx1;

                    throughput = camera.contribSensitivity(x0xV, x0xI, x0x1) * throughput;
                } else {
                    const double nowSampledPdfA = nowSampledPdfOmega * (toNextVertex.normalized().dot(orientNormal)) / toNextVertex.squaredNorm();
                    totalPdfA *= nowSampledPdfA;
                }

                // Geometry term
                const double G = toNextVertex.normalized().dot(orientNormal) * (-1.0 * toNextVertex).normalized().dot(prevNormal) / toNextVertex.squaredNorm();
                throughput *= G;

                if (min3(emittance.red(), emittance.green(), emittance.blue()) > 0.0) {
                    vertices->push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), isect.objectId(), Vertex::OBJECT_TYPE_LIGHT, totalPdfA, throughput));
                    const Color result = Color(throughput * emittance / totalPdfA);
                    return TraceResult(result, x, y, HIT_ON_LIGHT);
                }

                vertices->push_back(Vertex(hitpoint.position(), orientNormal, hitpoint.normal(), isect.objectId(),
                                           bsdf.type() == BsdfType::Lambertian ? Vertex::OBJECT_TYPE_DIFFUSE : Vertex::OBJECT_TYPE_SPECULAR,
                                           totalPdfA, throughput));

                if (bsdf.type() == BsdfType::Lambertian) {
                    Vector3D nextDir;
                    sampler::onHemisphere(orientNormal, &nextDir, rands[1], rands[2]);
                    nowSampledPdfOmega = sample_hemisphere_pdf_omega(orientNormal, nextDir);
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughput = bsdf.reflectance() * throughput * INV_PI;
                } else if (bsdf.type() == BsdfType::Specular) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3D nextDir = Vector3D::reflect(nowRay.direction(), hitpoint.normal());
                    nowRay = Ray(hitpoint.position(), nextDir);
                    throughput = bsdf.reflectance() * throughput / (toNextVertex.normalized().dot(orientNormal));

                } else if (bsdf.type() == BsdfType::Reflactive) {
                    const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                    Vector3D reflectdir, transdir;
                    double fresnelRe, fresnelTr;
                    bool totalReflection = helper::checkTotalReflection(isIncoming,
                                                                        nowRay.direction(), hitpoint.normal(), orientNormal,
                                                                        &reflectdir, &transdir, &fresnelRe, &fresnelTr);

                    if (totalReflection) {
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(hitpoint.position(), reflectdir);
                        throughput = bsdf.reflectance() * throughput / (toNextVertex.normalized().dot(orientNormal));                    
                    } else {
                        const double probability = 0.25 + 0.5 * kReflectProbability;
                        if (rands[1] < probability) {
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), reflectdir);
                            throughput = fresnelRe * bsdf.reflectance() * throughput / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= probability;
                        } else {
                            const double ratio = isIncoming ? kIorVaccum / kIorObject : kIorObject / kIorVaccum;
                            const double nnt2 = ratio * ratio;
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(hitpoint.position(), transdir);
                            throughput = (nnt2 * fresnelTr) * (bsdf.reflectance() * throughput) / (toNextVertex.normalized().dot(orientNormal));
                            totalPdfA *= (1.0 - probability);
                        }
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

        BPTResult executeBPT(const Scene& scene, const Camera& camera, Stack<double>& rstk, int x, int y, const int bounceLimit) {
            BPTResult bptResult;

            std::vector<Vertex> eyeVerts, lightVerts;
            const TraceResult ptResult = pathTrace(scene, camera, x, y, rstk, &eyeVerts, bounceLimit);
            const TraceResult ltResult = lightTrace(scene, camera, rstk, &lightVerts, bounceLimit);

            // If trace terminates on light, store path tracing result
            if (ptResult.hitObjType == HIT_ON_LIGHT) {
                const double weightMIS = calcMISWeight(scene, camera, eyeVerts[eyeVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, (const int)eyeVerts.size(), 0);
                const Color result = Color(weightMIS * ptResult.value);
                bptResult.samples.push_back(Sample(x, y, result, true));
            }

            // If trace terminates on lens, store light tracing result
            if (ltResult.hitObjType == HIT_ON_LENS) {
                const double weightMIS = calcMISWeight(scene, camera, lightVerts[lightVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, 0, (const int)lightVerts.size());
                const int lx = ltResult.imageX;
                const int ly = ltResult.imageY;
                const Color result = Color(weightMIS * ltResult.value);
                bptResult.samples.push_back(Sample(lx, ly, result, false));
            }

            // Connecting hitpoints of light/path tracing
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
                    Color connectedThroughput = Color(1.0, 1.0, 1.0);

                    if (lightVertId == 1) {
                        lightThrouput = scene.getEmittance(lightVerts[0].objectId);
                    }

                    // Cast ray from light-end to path-end to check existance of occluders
                    Intersection isect;
                    const Vector3D lendToEend = eyeEnd.position - lightEnd.position;
                    const Ray testRay(lightEnd.position, lendToEend.normalized());
                    scene.intersect(testRay, isect);

                    if (eyeEnd.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                        const double dist = (isect.hitpoint().position() - eyeEnd.position).norm();
                        if (dist >= EPS) {
                            continue;
                        }

                        const BSDF& eyeEndBsdf = scene.getBsdf(eyeEnd.objectId);
                        connectedThroughput = connectedThroughput * eyeEndBsdf.reflectance() * INV_PI;
                    } else if (eyeEnd.objtype == Vertex::OBJECT_TYPE_LENS) {
                        Vector3D positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor;
                        const double lensT = camera.intersectLens(testRay, positionOnLens, positionOnObjplane, positionOnSensor, uvOnSensor);
                        if (EPS < lensT && lensT < isect.hitpoint().distance()) {
                            const Vector3D x0xI = positionOnSensor - positionOnLens;
                            const Vector3D x0xV = positionOnObjplane - positionOnLens;
                            const Vector3D x0x1 = testRay.origin() - positionOnLens;

                            targetX = (int)uvOnSensor.x();
                            targetY = (int)uvOnSensor.y();
                            targetX = targetX < 0 ? 0 : targetX >= camera.imageW() ? camera.imageW() - 1 : targetX;
                            targetY = targetY < 0 ? 0 : targetY >= camera.imageH() ? camera.imageH() - 1 : targetY;

                            connectedThroughput *= camera.contribSensitivity(x0xV, x0xI, x0x1);
                        } else {
                            continue;
                        }
                    } else if (eyeEnd.objtype == Vertex::OBJECT_TYPE_LIGHT || eyeEnd.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                        continue;
                    }

                    if (lightEnd.objtype == Vertex::OBJECT_TYPE_DIFFUSE) {
                        const BSDF& tempBsdf = scene.getBsdf(lightEnd.objectId);
                        connectedThroughput = connectedThroughput * tempBsdf.reflectance() * INV_PI;
                    } else if (lightEnd.objtype == Vertex::OBJECT_TYPE_LIGHT) {

                    } else if (lightEnd.objtype == Vertex::OBJECT_TYPE_LENS || lightEnd.objtype == Vertex::OBJECT_TYPE_SPECULAR) {
                        continue;
                    }

                    double G = std::max(0.0, (-1.0 * lendToEend.normalized().dot(eyeEnd.orientNormal)));
                    G *= std::max(0.0, lendToEend.normalized().dot(lightEnd.orientNormal));
                    G /= lendToEend.dot(lendToEend);
                    connectedThroughput *= G;

                    const double weightMIS = calcMISWeight(scene, camera, totalPdfA, eyeVerts, lightVerts, eyeVertId, lightVertId);
                    if (isnan(weightMIS)) {
                        continue;
                    }

                    const Color result = Color(weightMIS * (connectedThroughput * eyeThoughput * lightThrouput) / totalPdfA);
                    bptResult.samples.push_back(Sample(targetX, targetY, result, eyeVertId > 1.0));
                }
            }

            return bptResult;
        }

        bool isInvalidValue(const Color& color) {
            if (isnan(color.red()) || isnan(color.green()) || isnan(color.blue())) return true;
            if (color.red() < 0.0 || INFTY < color.red()) return true;
            if (color.green() < 0.0 || INFTY < color.green()) return true;
            if (color.blue() < 0.0 || INFTY < color.blue()) return true;
            return false;
        }

        bool isValidValue(const Color& color) {
            return !isInvalidValue(color);
        }

    }  // unnamed namespace
    
    BDPTRenderer::BDPTRenderer(spica::Image* image)
        : IRenderer()
        , _image(image)
    {
    }

    BDPTRenderer::~BDPTRenderer()
    {
    }

    void BDPTRenderer::render(const Scene& scene, const Camera& camera, const RenderParameters& params) {
        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Prepare random samplers
        RandomSampler* samplers = new RandomSampler[kNumThreads];
        for (int i = 0; i < kNumThreads; i++) {
            switch (params.randomType()) {
            case PSEUDO_RANDOM_TWISTER:
                samplers[i] = Random::factory(i);
                break;

            case QUASI_MONTE_CARLO:
                samplers[i] = Halton::factory(250, true, i);
                break;

            default:
                std::cerr << "Unknown random number generator type!!" << std::endl;
                std::abort();
            }
        }
        
        Image* buffer = new Image[kNumThreads];
        for (int i = 0; i < kNumThreads; i++) {
            buffer[i] = Image(width, height);
        }

        bool isAllocInside = false;
        if (_image == NULL) {
            _image = new Image(width, height);
            isAllocInside = true;
        } else {
            _image->resize(width, height);
        }

        // Distribute tasks
        const int taskPerThread = (height + kNumThreads - 1) / kNumThreads;
        std::vector<std::vector<int> > tasks(kNumThreads);
        for (int y = 0; y < height; y++) {
            tasks[y % kNumThreads].push_back(y);
        }

        // Rendering
        for (int s = 0; s < params.samplePerPixel(); s++) {
            for (int t = 0; t < taskPerThread; t++) {
                ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                    Stack<double> rstk;
                    if (t < tasks[threadID].size()) {
                        const int y = tasks[threadID][t];
                        for (int x = 0; x < width; x++) {
                            samplers[threadID].request(&rstk, 250);
                            BPTResult bptResult = executeBPT(scene, camera, rstk, x, y, params.bounceLimit());
                    
                            for (int i = 0; i < bptResult.samples.size(); i++) {
                                const int ix = bptResult.samples[i].imageX;
                                const int iy = bptResult.samples[i].imageY;
                                if (isValidValue(bptResult.samples[i].value)) {
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

            _image->fill(Color(0.0, 0.0, 0.0));
            for (int k = 0; k < kNumThreads; k++) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        _image->pixel(width - x - 1, y) += buffer[k](x, y) / (s + 1);
                    }
                }
            }

            char filename[256];
            sprintf(filename, params.saveFilenameFormat().c_str(), s + 1);
            _image->gammaCorrect(1.0 / 2.2);
            _image->save(filename);

            printf("  %6.2f %%  processed -> %s\r", 100.0 * (s + 1) / params.samplePerPixel(), filename);
        }
        printf("\nFinish!!\n");

        delete[] samplers;
        delete[] buffer;

        if (isAllocInside) {
            delete _image;
            _image = NULL;
        }
    }

}
