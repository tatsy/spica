#define SPICA_API_EXPORT
#include "bdpt.h"

#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "renderer_helper.h"
#include "render_parameters.h"

#include "../core/common.h"
#include "../core/sampler.h"

#include "../image/tmo.h"

#include "../camera/dof_camera.h"
#include "../light/lighting.h"
#include "../bsdf/bsdf.h"

#include "../random/random_sampler.h"
#include "../random/random.h"
#include "../random/halton.h"

namespace spica {

    namespace {


        enum class HitOn : int {
            Light,
            Lens,
            Object
        };

        /** Structs for storing path/light tracing results.
         */
        struct TraceResult {
            Spectrum value;
            int imageX;
            int imageY;
            HitOn hitObj;

            TraceResult(const Spectrum& value_,
                        int imageX_, int imageY_,
                        HitOn hitObj_)
                : value{value_}
                , imageX{imageX_}
                , imageY{imageY_}
                , hitObj{hitObj_} {
            }
        };

        enum class ObjectType : int {
            Light,
            Lens,
            Diffuse,
            Dielectric
        };

        /* Struct for storing traced vertices
         */
        struct Vertex {
            Point position;
            Normal objectNormal;
            Normal orientNormal;
            Spectrum reflectance;
            Spectrum emission;
            ObjectType objtype;
            double totalPdfA;
            Spectrum throughput;
            int objectID;

            Vertex(const Point& position_,
                   const Normal& orientNormal_,
                   const Normal& objectNormal_,
                   const Spectrum& reflectance_,
                   const Spectrum& emission_,
                   const ObjectType objtype_,
                   const double totalPdfA_,
                   const Spectrum& throughput_,
                   int objectID_)
                : position{position_}
                , orientNormal{orientNormal_}
                , objectNormal{objectNormal_}
                , reflectance{reflectance_}
                , emission{emission_}
                , objtype{objtype_}
                , totalPdfA{totalPdfA_}
                , throughput{throughput_}
                , objectID{objectID_} {
            }
        };


        // --------------------------------------------------
        // Sample functions
        // --------------------------------------------------

        double sample_hemisphere_pdf_omega(const Normal& normal, const Vector3D& direction) {
            return std::max(vect::dot(normal, direction), 0.0) * INV_PI;
        }

        // --------------------------------------------------
        // Multiple importance sampling
        // --------------------------------------------------

        double calcPdfA(const Scene& scene, const DoFCamera& camera, const std::vector<const Vertex*>& verts, const int prevFromIdx, const int fromIdx, const int nextIdx) {
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

            if (fromVert.objtype == ObjectType::Light ||
                fromVert.objtype == ObjectType::Diffuse) {
                pdf = sample_hemisphere_pdf_omega(fromVert.orientNormal, normalizedTo);
            } else if (fromVert.objtype == ObjectType::Lens) {
                const Ray testRay(nextVert.position, -normalizedTo);
                Point positionOnLens, positionOnObjplane, positionOnSensor;
                Vector3D uvOnSensor;
                const double lensT = camera.intersectLens(testRay, &positionOnLens, &positionOnObjplane, &positionOnSensor, &uvOnSensor);
                if (EPS < lensT) {
                    const Vector3D x0xI = positionOnSensor - positionOnLens;
                    const Vector3D x0xV = positionOnObjplane - positionOnLens;
                    const Vector3D x0x1 = testRay.origin() - positionOnLens;

                    const double PImage = 1.0 / (camera.sensorW() * camera.sensorH());
                    const double PAx1 = camera.PImageToPAx1(PImage, x0xV, x0x1, nextVert.orientNormal);
                    return PAx1;
                }
                return 0.0;
            } else if (fromVert.objtype == ObjectType::Dielectric) {
                const BSDF& bsdf = scene.getBsdf(fromVert.objectID);
                if (bsdf.type() == BsdfType::Refractive) {
                    if (prevFromVertex != nullptr) {
                        const Vector3D intoFromVertexDir = (fromVert.position - prevFromVertex->position).normalized();
                        const bool isIncoming = vect::dot(intoFromVertexDir, fromVert.objectNormal) < 0.0;
                        const Normal fromNewOrientNormal = isIncoming ? fromVert.objectNormal : -fromVert.objectNormal;

                        Vector3D reflectdir, transdir;
                        double fresnelRe, fresnelTr;
                        bool totalReflection = helper::checkTotalReflection(isIncoming,
                                                                            intoFromVertexDir, fromVert.objectNormal, fromNewOrientNormal,
                                                                             &reflectdir, &transdir, &fresnelRe, &fresnelTr);
                        if (totalReflection) {
                            pdf = 1.0;
                        } else {
                            pdf = vect::dot(fromNewOrientNormal, normalizedTo) > 0.0 ? reflectProb : 1.0 - reflectProb;
                        }
                    }
                } else {
                    pdf = 1.0;
                }
            }

            const Normal nextNewOrientNormal = vect::dot(to, nextVert.objectNormal) < 0.0 ? nextVert.objectNormal : -nextVert.objectNormal;
            return pdf * vect::dot(-normalizedTo, nextNewOrientNormal) / to.dot(to);
        }

        double calcMISWeight(const Scene& scene, const DoFCamera& camera, const double totalPdfA, const std::vector<Vertex>& eyeVerts, const std::vector<Vertex>& lightVerts, const int nEyeVerts, const int nLightVerts) {
            std::vector<const Vertex*> verts(nEyeVerts + nLightVerts);
            std::vector<double> pi1pi(nEyeVerts + nLightVerts);

            const double PAy0 = 1.0 / scene.lightArea();
            const double PAx0 = 1.0 / camera.lensArea();

            const int k = nEyeVerts + nLightVerts - 1;
            for (int i = 0; i < nLightVerts; i++) {
                verts[i] = &lightVerts[i];
            }
            for (int i = nEyeVerts - 1; i >= 0; i--) {
                verts[nLightVerts + nEyeVerts - i - 1] = &eyeVerts[i]; 
            }

            // Russian roulette probability
            double roulette = 0.0;
            if (verts[0]->emission.norm() > 0.0) {
                roulette = 1.0;
            } else if (verts[0]->objectID >= 0) {
                const Spectrum& refl = verts[0]->reflectance;
                roulette = std::min(1.0, max3(refl.red(), refl.green(), refl.blue()));
            }

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
                if (verts[i]->objtype == ObjectType::Dielectric) {
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

        TraceResult lightTrace(const Scene& scene, const DoFCamera& camera, Stack<double>& rstk, std::vector<Vertex>* vertices, const int bounceLimit) {
            // Generate sample on the light
            const LightSample Ls = scene.sampleLight(Point(), rstk);

            // Store vertex on light itself
            double totalPdfA = 1.0 / scene.lightArea();
            vertices->push_back(Vertex(Ls.position(), Ls.normal(), Ls.normal(),
                                       Spectrum(0.0, 0.0, 0.0), Ls.Le(), ObjectType::Light, totalPdfA, Spectrum(0.0, 0.0, 0.0), -1));

            // Compute initial tracing direction
            Vector3D nextDir;
            sampler::onHemisphere(Ls.normal(), &nextDir, rstk.pop(), rstk.pop());
            double pdfOmega = sample_hemisphere_pdf_omega(Ls.normal(), nextDir);

            Ray currentRay(Ls.position(), nextDir);
            Normal prevNormal = Ls.normal();

            // Trace light ray
            Spectrum throughput = Ls.Le();
            for (int bounce = 0; bounce < bounceLimit; bounce++) {
                const double rands[3] = { rstk.pop(), rstk.pop(), rstk.pop() };

                Intersection isect;
                const bool isHitScene = scene.intersect(currentRay, &isect);

                // If ray hits on the lens, return curent result
                Point positionOnLens, positionOnObjplane, positionOnSensor;
                Vector3D uvOnSensor;
                double lensT = camera.intersectLens(currentRay, &positionOnLens, &positionOnObjplane, &positionOnSensor, &uvOnSensor);
                if (EPS < lensT && lensT < isect.distance()) {
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
                    const double G = x0x1.normalized().dot(camera.direction().normalized()) * (-1.0) * vect::dot(x0x1.normalized(), prevNormal) / x0x1.dot(x0x1);
                    throughput *= G;
                    vertices->push_back(Vertex(positionOnLens, Normal(camera.direction().normalized()), Normal(camera.direction().normalized()),
                                        Spectrum(0.0, 0.0, 0.0), Spectrum(0.0, 0.0, 0.0), ObjectType::Lens, totalPdfA, throughput, -1));
                
                    const Spectrum result = Spectrum((camera.contribSensitivity(x0xV, x0xI, x0x1) * throughput) / totalPdfA);
                    return TraceResult(result, x, y, HitOn::Lens);
                }

                if (!isHitScene) {
                    break;
                }

                // Otherwise, trace next direction
                const int triangleID = isect.objectID();
                const BSDF& bsdf = scene.getBsdf(triangleID);
                const Spectrum& refl = isect.color();

                const Normal orientNormal = vect::dot(isect.normal(), currentRay.direction()) < 0.0 ? isect.normal() : -isect.normal();
                const double rouletteProb = scene.isLightCheck(triangleID) ? 1.0 : std::min(1.0, max3(refl.red(), refl.green(), refl.blue()));
                
                if (rands[0] >= rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3D toNextVertex = currentRay.origin() - isect.position();
                const double nowSampledPdfArea = pdfOmega * vect::dot(toNextVertex.normalized(), orientNormal) / Vector3D::dot(toNextVertex, toNextVertex);
                totalPdfA *= nowSampledPdfArea;

                const double G = vect::dot(toNextVertex.normalized(), orientNormal) * vect::dot((-1.0 * toNextVertex).normalized(), prevNormal) / toNextVertex.dot(toNextVertex);
                throughput *= G;

                Spectrum emission = Spectrum(0.0, 0.0, 0.0);
                if (scene.isLightCheck(triangleID)) {
                    emission = scene.directLight(currentRay.direction());
                }
                vertices->push_back(Vertex(isect.position(), orientNormal, isect.normal(), isect.color(), emission,
                                          bsdf.type() == BsdfType::Lambertian ? ObjectType::Diffuse : ObjectType::Dielectric,
                                          totalPdfA, throughput, isect.objectID()));

                if (bsdf.type() == BsdfType::Lambertian) {
                    sampler::onHemisphere(orientNormal, &nextDir, rands[1], rands[2]);
                    pdfOmega = sample_hemisphere_pdf_omega(orientNormal, nextDir);
                    currentRay = Ray(isect.position(), nextDir);
                    throughput = isect.color() * throughput * INV_PI;
                } else if (bsdf.type() == BsdfType::Specular) {
                    pdfOmega = 1.0;
                    const Vector3D nextDir = vect::reflect(currentRay.direction(), isect.normal());
                    currentRay = Ray(isect.position(), nextDir);
                    throughput = isect.color() * throughput / vect::dot(toNextVertex.normalized(), orientNormal);
                } else if (bsdf.type() == BsdfType::Refractive) {
                    const bool isIncoming = vect::dot(isect.normal(), orientNormal) > 0.0;

                    Vector3D reflectdir, transdir;
                    double fresnelRe, fresnelTr;
                    bool totalReflection = helper::checkTotalReflection(isIncoming,
                                                                        currentRay.direction(), isect.normal(), orientNormal,
                                                                        &reflectdir, &transdir, &fresnelRe, &fresnelTr);

                    if (totalReflection) {
                        pdfOmega = 1.0;
                        currentRay = Ray(isect.position(), reflectdir);
                        throughput = isect.color() * throughput / vect::dot(toNextVertex.normalized(), orientNormal);
                    } else {
                        const double probability = 0.25 + 0.5 * kReflectProbability;
                        if (rands[1] < probability) {
                            pdfOmega = 1.0;
                            currentRay = Ray(isect.position(), reflectdir);
                            throughput = fresnelRe * (isect.color() * throughput) / vect::dot(toNextVertex.normalized(), orientNormal);
                            totalPdfA *= probability;
                        } else {
                            const double ratio = isIncoming ? kIorVaccum / kIorObject : kIorObject / kIorVaccum;
                            const double nnt2   = ratio * ratio;
                            pdfOmega = 1.0;
                            currentRay = Ray(isect.position(), transdir);
                            throughput = (nnt2 * fresnelTr) * (isect.color() * throughput) / vect::dot(toNextVertex.normalized(), orientNormal);
                            totalPdfA *= (1.0 - probability);
                        }
                    }
                }
                prevNormal = orientNormal;
            }

            return TraceResult(Spectrum(), 0, 0, HitOn::Object);
        }

        TraceResult pathTrace(const Scene& scene, const DoFCamera& camera, int x, int y, Stack<double>& rstk, std::vector<Vertex>* vertices, const int bounceLimit) {
            // Sample point on lens and object plane
            CameraSample camSample = camera.sample(x, y, rstk);

            double totalPdfA = 1.0 / camera.lensArea();

            Spectrum throughput = Spectrum(1.0, 1.0, 1.0);

            vertices->push_back(Vertex(camSample.posLens(), camera.lensNormal(), camera.lensNormal(),
                                       Spectrum(0.0, 0.0, 0.0), Spectrum(0.0, 0.0, 0.0), ObjectType::Lens, totalPdfA, throughput, -1));
        
            Ray nowRay = camSample.ray();
            double nowSampledPdfOmega = 1.0;
            Normal prevNormal = camera.lensNormal();

            for (int bounce = 0; bounce < bounceLimit; bounce++) {
                // Get next random
                const double rands[3] = { rstk.pop(), rstk.pop(), rstk.pop() };

                Intersection isect;
                if (!scene.intersect(nowRay, &isect)) {
                    break;
                }

                const BSDF& bsdf = scene.getBsdf(isect.objectID());
                const Spectrum& refl = isect.color();

                const Normal orientNormal = vect::dot(isect.normal(), nowRay.direction()) < 0.0 ? isect.normal() : -isect.normal();
                const double rouletteProb = scene.isLightCheck(isect.objectID()) ? 1.0 : std::min(1.0, max3(refl.red(), refl.green(), refl.blue()));

                if (rands[0] > rouletteProb) {
                    break;
                }

                totalPdfA *= rouletteProb;

                const Vector3D toNextVertex = nowRay.origin() - isect.position();
                if (bounce == 0) {
                    const Vector3D x0xI = camSample.posSensor() - camSample.posLens();
                    const Vector3D x0xV = camSample.posObjplane() - camSample.posLens();
                    const Vector3D x0x1 = isect.position() - camSample.posLens();
                    const double pdfImage = 1.0 / (camera.cellW() * camera.cellH());
                    const double PAx1 = camera.PImageToPAx1(pdfImage, x0xV, x0x1, orientNormal);
                    totalPdfA *= PAx1;

                    throughput = camera.contribSensitivity(x0xV, x0xI, x0x1) * throughput;
                } else {
                    const double nowSampledPdfA = nowSampledPdfOmega * vect::dot(toNextVertex.normalized(), orientNormal) / toNextVertex.squaredNorm();
                    totalPdfA *= nowSampledPdfA;
                }

                // Geometry term
                const double G = vect::dot(toNextVertex.normalized(), orientNormal) * vect::dot((-1.0 * toNextVertex).normalized(), prevNormal) / toNextVertex.squaredNorm();
                throughput *= G;

                if (scene.isLightCheck(isect.objectID())) {
                    const Spectrum emittance = scene.directLight(nowRay.direction());
                    const Spectrum result = Spectrum(throughput * emittance / totalPdfA);
                    vertices->push_back(Vertex(isect.position(), orientNormal, isect.normal(),
                                               isect.color(), emittance, ObjectType::Light, totalPdfA, throughput, isect.objectID()));
                    return TraceResult(result, x, y, HitOn::Light);
                }

                vertices->push_back(Vertex(isect.position(), orientNormal, isect.normal(), isect.color(), Spectrum(0.0, 0.0, 0.0),
                                           bsdf.type() == BsdfType::Lambertian ? ObjectType::Diffuse : ObjectType::Dielectric,
                                           totalPdfA, throughput, isect.objectID()));

                if (bsdf.type() == BsdfType::Lambertian) {
                    Vector3D nextDir;
                    sampler::onHemisphere(orientNormal, &nextDir, rands[1], rands[2]);
                    nowSampledPdfOmega = sample_hemisphere_pdf_omega(orientNormal, nextDir);
                    nowRay = Ray(isect.position(), nextDir);
                    throughput = refl * throughput * INV_PI;
                } else if (bsdf.type() == BsdfType::Specular) {
                    nowSampledPdfOmega = 1.0;
                    const Vector3D nextDir = vect::reflect(nowRay.direction(), isect.normal());
                    nowRay = Ray(isect.position(), nextDir);
                    throughput = refl * throughput / vect::dot(toNextVertex.normalized(), orientNormal);

                } else if (bsdf.type() == BsdfType::Refractive) {
                    const bool isIncoming = vect::dot(isect.normal(), orientNormal) > 0.0;

                    Vector3D reflectdir, transdir;
                    double fresnelRe, fresnelTr;
                    bool totalReflection = helper::checkTotalReflection(isIncoming,
                                                                        nowRay.direction(), isect.normal(), orientNormal,
                                                                        &reflectdir, &transdir, &fresnelRe, &fresnelTr);

                    if (totalReflection) {
                        nowSampledPdfOmega = 1.0;
                        nowRay = Ray(isect.position(), reflectdir);
                        throughput = refl * throughput / vect::dot(toNextVertex.normalized(), orientNormal);                    
                    } else {
                        const double probability = 0.25 + 0.5 * kReflectProbability;
                        if (rands[1] < probability) {
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(isect.position(), reflectdir);
                            throughput = fresnelRe * refl * throughput / vect::dot(toNextVertex.normalized(), orientNormal);
                            totalPdfA *= probability;
                        } else {
                            const double ratio = isIncoming ? kIorVaccum / kIorObject : kIorObject / kIorVaccum;
                            const double nnt2 = ratio * ratio;
                            nowSampledPdfOmega = 1.0;
                            nowRay = Ray(isect.position(), transdir);
                            throughput = (nnt2 * fresnelTr) * (refl * throughput) / vect::dot(toNextVertex.normalized(), orientNormal);
                            totalPdfA *= (1.0 - probability);
                        }
                    }
                }
                prevNormal = orientNormal;
            }

            return TraceResult(Spectrum(), 0, 0, HitOn::Object);
        }

        struct Sample {
            int imageX;
            int imageY;
            Spectrum value;
            bool startFromPixel;

            Sample(const int imageX_, const int imageY_, const Spectrum& value_, const bool startFromPixel_)
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

        BPTResult executeBPT(const Scene& scene, const DoFCamera& camera, Stack<double>& rstk, int x, int y, const int bounceLimit) {
            BPTResult bptResult;

            std::vector<Vertex> eyeVerts, lightVerts;
            const TraceResult ptResult = pathTrace(scene, camera, x, y, rstk, &eyeVerts, bounceLimit);
            const TraceResult ltResult = lightTrace(scene, camera, rstk, &lightVerts, bounceLimit);

            // If trace terminates on light, store path tracing result
            if (ptResult.hitObj == HitOn::Light) {
                const double weightMIS = calcMISWeight(scene, camera, eyeVerts[eyeVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, (const int)eyeVerts.size(), 0);
                const Spectrum result = Spectrum(weightMIS * ptResult.value);
                bptResult.samples.push_back(Sample(x, y, result, true));
            }

            // If trace terminates on lens, store light tracing result
            if (ltResult.hitObj == HitOn::Lens) {
                const double weightMIS = calcMISWeight(scene, camera, lightVerts[lightVerts.size() - 1].totalPdfA, eyeVerts, lightVerts, 0, (const int)lightVerts.size());
                const int lx = ltResult.imageX;
                const int ly = ltResult.imageY;
                const Spectrum result = Spectrum(weightMIS * ltResult.value);
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

                    Spectrum eyeThoughput = eyeEnd.throughput;
                    Spectrum lightThrouput = lightEnd.throughput;
                    Spectrum connectedThroughput = Spectrum(1.0, 1.0, 1.0);

                    if (lightVertId == 1) {
                        lightThrouput = lightVerts[0].emission;
                    }

                    // Cast ray from light-end to path-end to check existance of occluders
                    Intersection isect;
                    const Vector3D lendToEend = eyeEnd.position - lightEnd.position;
                    const Ray testRay(lightEnd.position, lendToEend.normalized());
                    scene.intersect(testRay, &isect);

                    if (eyeEnd.objtype == ObjectType::Diffuse) {
                        const double dist = (isect.position() - eyeEnd.position).norm();
                        if (dist >= EPS) {
                            continue;
                        }

                        connectedThroughput = connectedThroughput * eyeEnd.reflectance * INV_PI;
                    } else if (eyeEnd.objtype == ObjectType::Lens) {
                        Point positionOnLens, positionOnObjplane, positionOnSensor;
                        Vector3D uvOnSensor;
                        const double lensT = camera.intersectLens(testRay, &positionOnLens, &positionOnObjplane, &positionOnSensor, &uvOnSensor);
                        if (EPS < lensT && lensT < isect.distance()) {
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
                    } else if (eyeEnd.objtype == ObjectType::Light ||
                               eyeEnd.objtype == ObjectType::Dielectric) {
                        continue;
                    }

                    if (lightEnd.objtype == ObjectType::Diffuse) {
                        connectedThroughput = connectedThroughput * lightEnd.reflectance * INV_PI;
                    } else if (lightEnd.objtype == ObjectType::Light) {

                    } else if (lightEnd.objtype == ObjectType::Lens || 
                               lightEnd.objtype == ObjectType::Dielectric) {
                        continue;
                    }

                    double G = std::max(0.0, (-1.0 * vect::dot(lendToEend.normalized(), eyeEnd.orientNormal)));
                    G *= std::max(0.0, vect::dot(lendToEend.normalized(), lightEnd.orientNormal));
                    G /= lendToEend.dot(lendToEend);
                    connectedThroughput *= G;

                    const double weightMIS = calcMISWeight(scene, camera, totalPdfA, eyeVerts, lightVerts, eyeVertId, lightVertId);
                    if (isnan(weightMIS)) {
                        continue;
                    }

                    const Spectrum result = Spectrum(weightMIS * (connectedThroughput * eyeThoughput * lightThrouput) / totalPdfA);
                    bptResult.samples.push_back(Sample(targetX, targetY, result, eyeVertId > 1.0));
                }
            }

            return bptResult;
        }

        bool isInvalidValue(const Spectrum& color) {
            if (isnan(color.red()) || isnan(color.green()) || isnan(color.blue())) return true;
            if (color.red() < 0.0 || INFTY < color.red()) return true;
            if (color.green() < 0.0 || INFTY < color.green()) return true;
            if (color.blue() < 0.0 || INFTY < color.blue()) return true;
            return false;
        }

        bool isValidValue(const Spectrum& color) {
            return !isInvalidValue(color);
        }

    }  // anonymous namespace
    
    BDPTRenderer::BDPTRenderer()
        : IRenderer{RendererType::BDPT} {
    }

    BDPTRenderer::~BDPTRenderer() {
    }

    void BDPTRenderer::render(const Scene& scene, const Camera& camera, 
                              const RenderParameters& params) {
        // Currently, BDPT renderer only supports DoF camera
        Assertion(camera.type() == CameraType::DepthOfField,
                  "camera for BDPT must be DoF type!!");

        const int width  = camera.imageW();
        const int height = camera.imageH();

        // Prepare random samplers
        std::vector<RandomSampler> samplers(kNumThreads);
        for (int i = 0; i < kNumThreads; i++) {
            switch (params.randomType()) {
            case RandomType::MT19937:
                samplers[i] = RandomSampler::useMersenne(i);
                break;

            case RandomType::Halton:
                samplers[i] = RandomSampler::useHalton(250, true, i);
                break;

            default:
                FatalError("[ERROR] unknown random number generator type!!");
            }
        }
        
        std::vector<Image> buffer(kNumThreads, Image(width, height));

        _result.resize(width, height);

        // Distribute tasks
        const int taskPerThread = (height + kNumThreads - 1) / kNumThreads;
        std::vector<std::vector<int> > tasks(kNumThreads);
        for (int y = 0; y < height; y++) {
            tasks[y % kNumThreads].push_back(y);
        }

        // Rendering
        DoFCamera* dofCam = reinterpret_cast<DoFCamera*>(camera.getPtr());
        for (int s = 0; s < params.samplePerPixel(); s++) {
            for (int t = 0; t < taskPerThread; t++) {
                ompfor (int threadID = 0; threadID < kNumThreads; threadID++) {
                    Stack<double> rstk;
                    if (t < tasks[threadID].size()) {
                        const int y = tasks[threadID][t];
                        for (int x = 0; x < width; x++) {
                            renderPixel(scene, (*dofCam), params,
                                        samplers[threadID],
                                        buffer[threadID], x, y);
                        }
                    }
                }
            }

            _result.fill(Spectrum(0.0, 0.0, 0.0));
            for (int k = 0; k < kNumThreads; k++) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        const Spectrum pix = buffer[k](x, y) / (s + 1);
                        _result.pixel(width - x - 1, y) += pix;
                    }
                }
            }

            char filename[256];
            sprintf(filename, params.saveFilenameFormat().c_str(), s + 1);
            _result = GammaTmo(2.0).apply(_result);
            _result.save(filename);

            printf("  %6.2f %%  processed -> %s\r",
                   100.0 * (s + 1) / params.samplePerPixel(), filename);
        }
        printf("\nFinish!!\n");
    }

    void BDPTRenderer::renderPixel(const Scene& scene, const DoFCamera& camera,
                                   const RenderParameters& params,
                                   RandomSampler& sampler,
                                   Image& buffer, int x, int y) const {
        const int width = camera.imageW();
        const int height = camera.imageH();
        
        Stack<double> rstk;
        sampler.request(&rstk, 250);
        BPTResult bptResult = executeBPT(scene, camera, rstk,
                                         x, y, params.bounceLimit());

        for (int i = 0; i < bptResult.samples.size(); i++) {
            const int ix = bptResult.samples[i].imageX;
            const int iy = bptResult.samples[i].imageY;
            if (isValidValue(bptResult.samples[i].value)) {
                if (bptResult.samples[i].startFromPixel) {
                    buffer.pixel(ix, iy) += bptResult.samples[i].value;
                }
                else {
                    const auto div = static_cast<double>(width * height);
                    buffer.pixel(ix, iy) += bptResult.samples[i].value / div;
                }
            }
        }
    }

}  // namespace spica
