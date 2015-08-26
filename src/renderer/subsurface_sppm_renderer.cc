#define SPICA_SUBSURFACE_SPPM_RENDERER_EXPORT
#include "subsurface_sppm_renderer.h"

#include <cstdio>

#include "renderer_constants.h"
#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    const double SubsurfaceSPPMRenderer::ALPHA = 0.8;

    SubsurfaceSPPMRenderer::SubsurfaceSPPMRenderer(Image* image)
        : _image(image)
    {
    }

    SubsurfaceSPPMRenderer::~SubsurfaceSPPMRenderer()
    {
    }

    void SubsurfaceSPPMRenderer::render(const Scene& scene, const Camera& camera, const BSSRDF& bssrdf, const int samplePerPixel, const int numPhotons, const RandomType randType) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        const int numPixels = width * height;
        const double areaRadius = 0.1;

        // Prepare hit points for image pixels
        std::vector<HitpointInfo> hpoints(numPixels);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const int idx = y * width + x;
                HitpointInfo& hp = hpoints[idx];
                hp.imageX = x;
                hp.imageY = y;
                hp.n = 0;
            }
        }

        // Prepare random number generator
        RandomBase* rand = NULL;
        switch (randType) {
        case PSEUDO_RANDOM_TWISTER:
            rand = new Random();
            break;
        case QUASI_MONTE_CARLO:
            rand = new Halton();
            break;
        default:
            Assertion(false, "Unknown random number generator type!!");
            break;
        }

        bool isAllocImageInside = false;
        if (_image == NULL) {
            _image = new Image(width, height);
            isAllocImageInside = true;
        } else {
            _image->resize(width, height);
        }

        // Main rendering
        const int numPoints = static_cast<int>(hpoints.size());
        for (int t = 0; t < samplePerPixel; t++) {
            std::cout << "--- Iteration No." << (t + 1) << " ---" << std::endl;

            // 0th pass: Compute irradiacne for SSS object
            integrator.initialize(scene, bssrdf, PMParams(numPhotons), areaRadius, randType);
            
            // 1st pass: Trace rays from camera
            traceRays(scene, camera, rand, hpoints);

            // 2nd pass: Trace photons from light source
            tracePhotons(scene, rand, numPhotons);

            // Save temporal image
            _image->fill(Color(0.0, 0.0, 0.0));
            for (int i = 0; i < numPoints; i++) {
                const HitpointInfo& hp = hpoints[i];
                if (hp.imageX >= 0 && hp.imageY >= 0 && hp.imageX < width && hp.imageY < height) {
                    _image->pixel(width - hp.imageX - 1, hp.imageY) += (hp.emission + hp.flux / (PI * hp.r2)) * (hp.coeff / (t + 1));
                }
            }

            char filename[256];
            sprintf(filename, "sss_sppm_%02d.bmp", t + 1);
            _image->gamma(2.2, true);
            _image->saveBMP(filename);
        }

        // Release memories
        delete rand;
        if (isAllocImageInside) {
            delete _image;
        }
    }

    void SubsurfaceSPPMRenderer::constructHashGrid(std::vector<HitpointInfo>& hpoints, const int imageW, const int imageH) {

        // Clear current data
        hashgrid.clear();

        const int numPoints = static_cast<int>(hpoints.size());

        // Compute bounding box
        BBox bbox;
        for (int i = 0; i < numPoints; i++) {
            bbox.merge(static_cast<Vector3D>(hpoints[i]));
        }

        // Heuristic for initial radius
        Vector3D boxSize = bbox.posMax() - bbox.posMin();
        const double irad = ((boxSize.x() + boxSize.y() + boxSize.z()) / 3.0) / ((imageW + imageH) / 2.0) * 2.0;

        // Update initial radius
        Vector3D iradv(irad, irad, irad);
        for (int i = 0; i < numPoints; i++) {
            if (hpoints[i].n == 0) {
                hpoints[i].r2 = irad * irad;
                hpoints[i].n = 0;
                hpoints[i].flux = Color(0.0, 0.0, 0.0);
            }

            bbox.merge(hpoints[i] + iradv);
            bbox.merge(hpoints[i] - iradv);
        }
        
        // Make each grid cell two times larger than the initial radius
        const double hashScale = 1.0 / (irad * 2.0);
        const int hashSize = numPoints;

        hashgrid.init(hashSize, hashScale, bbox);

        // Set hit points to the grid
        for (int i = 0; i < numPoints; i++) {
            Vector3D boxMin = static_cast<Vector3D>(hpoints[i]) - iradv;
            Vector3D boxMax = static_cast<Vector3D>(hpoints[i]) + iradv;
            hashgrid.add(&hpoints[i], boxMin, boxMax);
        }
    }

    void SubsurfaceSPPMRenderer::traceRays(const Scene& scene, const Camera& camera, RandomBase* rand, std::vector<HitpointInfo>& hpoints) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        const int numPoints = static_cast<int>(hpoints.size());

        // Generate a ray to cast
        std::cout << "Tracing rays from camera ..." << std::endl;

        // Distribute hitpoints to each thread
        std::vector<std::vector<int> > hpIDs(OMP_NUM_CORE);
        for (int i = 0; i < numPoints; i++) {
            hpIDs[i % OMP_NUM_CORE].push_back(i);
        }

        int proc = 0;
        ompfor (int threadID = 0; threadID < OMP_NUM_CORE; threadID++) {
            const int taskPerThread = static_cast<int>(hpIDs[threadID].size());
            for (int i = 0; i < taskPerThread; i++) {
                RandomSeq rseq;
                omplock {
                    rand->requestSamples(rseq, 200);
                }

                const int hpID = hpIDs[threadID][i];
                executePathTracing(scene, camera, rseq, &hpoints[hpID]);

                omplock {
                    proc += 1;
                    if (proc % width == 0) {
                        printf("%6.2f %% processed...\r", 100.0 * proc / numPoints);
                    }
                }
            }
        }
        printf("\nFinish !!\n");

        // Construct k-d tree
        constructHashGrid(hpoints, width, height);
        std::cout << "Hash grid constructed !!" << std::endl << std::endl;
    }

    void SubsurfaceSPPMRenderer::tracePhotons(const Scene& scene, RandomBase* rand, const int numPhotons, const int bounceLimit) {
        std::cout << "Shooting photons ..." << std::endl;
        int proc = 0;
        ompfor (int pid = 0; pid < numPhotons; pid++) {
            RandomSeq rseq;
            omplock {
                rand->requestSamples(rseq, 200);
            }

            Photon photon = Photon::sample(scene, rseq, numPhotons);
            const Vector3D& posOnLight = static_cast<Vector3D>(photon);
            const Vector3D& normalOnLight = photon.normal();

            // Compute flux
            Color currentFlux = photon.flux();

            // Prepare ray
            const double r1 = rseq.next();
            const double r2 = rseq.next();
            Vector3D nextDir;
            sampler::onHemisphere(normalOnLight, &nextDir, r1, r2);
            Ray currentRay(posOnLight, nextDir);
            Vector3D prevNormal = normalOnLight;

            // Shooting photons
            for (int bounce = 0; ; bounce++) {
                std::vector<double> randnums;
                rseq.next(3, &randnums);

                // Remove photons with zero flux
                if (std::max(currentFlux.red(), std::max(currentFlux.green(), currentFlux.blue())) <= 0.0 || bounce >= bounceLimit) {
                    break;
                }

                // Intersection check
                Intersection isect;
                bool isHit = scene.intersect(currentRay, isect);
                if (!isHit) {
                    break;
                }

                const int objectID = isect.objectId();
                const Material& mtrl = scene.getMaterial(objectID);
                const Hitpoint& hitpoint = isect.hitpoint();
                const Vector3D orientNormal = Vector3D::dot(hitpoint.normal(), currentRay.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    // Photon reaches diffuse surface. Update hitpoints.

                    // Gather hit points
                    std::vector<HitpointInfo*> results;
                    omplock{
                        results = hashgrid[hitpoint.position()];
                    }

                    // Update hit points
                    for (int i = 0; i < results.size(); i++) {
                        HitpointInfo* hpp = results[i];
                        Vector3D v = (*hpp) - hitpoint.position();
                        if (Vector3D::dot(hpp->normal, hitpoint.normal()) > EPS && (v.squaredNorm() <= hpp->r2)) {
                            double g = (hpp->n * ALPHA + ALPHA) / (hpp->n * ALPHA + 1.0);
                            omplock{
                                hpp->r2 *= g;
                                hpp->n += 1;
                                hpp->flux = (hpp->flux + hpp->weight.multiply(currentFlux) * (1.0 / PI)) * g;
                            }
                        }
                    }

                    // Determine continue or terminate trace with Russian roulette
                    const double probability = (mtrl.color.red() + mtrl.color.green() + mtrl.color.blue()) / 3.0;
                    if (randnums[0] < probability) {
                        sampler::onHemisphere(orientNormal, &nextDir, randnums[1], randnums[2]);
                        currentRay = Ray(hitpoint.position(), nextDir);
                        currentFlux = currentFlux.multiply(mtrl.color) / probability;
                    } else {
                        break;
                    }

                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nextDir = Vector3D::reflect(currentRay.direction(), hitpoint.normal());
                    currentRay = Ray(hitpoint.position(), nextDir);
                    currentFlux = currentFlux.multiply(mtrl.color);
                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    bool isIncoming = Vector3D::dot(hitpoint.normal(), orientNormal) > 0.0;
                    Vector3D reflectDir, refractDir;
                    double fresnelRe, fresnelTr;
                    bool isTotRef = helper::isTotalRef(isIncoming,
                                                       hitpoint.position(),
                                                       currentRay.direction(),
                                                       hitpoint.normal(),
                                                       orientNormal,
                                                       &reflectDir,
                                                       &refractDir,
                                                       &fresnelRe,
                                                       &fresnelTr);
                    if (isTotRef) {
                        // Total reflection
                        currentRay = Ray(hitpoint.position(), reflectDir);
                        currentFlux = currentFlux.multiply(mtrl.color);
                    } else {
                        // Trace either of reflect and transmit rays
                        const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                        if (randnums[0] < probability) {
                            // Reflect
                            currentRay = Ray(hitpoint.position(), reflectDir);
                            currentFlux = currentFlux.multiply(mtrl.color) * (fresnelRe / probability);
                        } else {
                            // Transmit
                            currentRay = Ray(hitpoint.position(), refractDir);
                            currentFlux = currentFlux.multiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                        }
                    }
                } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
                    bool isIncoming = Vector3D::dot(hitpoint.normal(), orientNormal) > 0.0;
                    Vector3D reflectDir, refractDir;
                    double fresnelRe, fresnelTr;
                    bool isTotRef = helper::isTotalRef(isIncoming,
                                                       hitpoint.position(),
                                                       currentRay.direction(),
                                                       hitpoint.normal(),
                                                       orientNormal,
                                                       &reflectDir,
                                                       &refractDir,
                                                       &fresnelRe,
                                                       &fresnelTr);
                    if (isTotRef) {
                        // Total reflection
                        currentRay = Ray(hitpoint.position(), reflectDir);
                        currentFlux = currentFlux.multiply(mtrl.color);
                    } else {
                        // Reflect
                        currentRay = Ray(hitpoint.position(), reflectDir);
                        currentFlux = currentFlux.multiply(mtrl.color) * fresnelRe;
                    }
                }
            }

            omplock{
                proc += 1;
                if (proc % 100 == 0) {
                    printf("%6.2f %% processed ...\r", 100.0 * proc / numPhotons);
                }
            }
        }
        printf("\nFinish !!\n\n");
    }

    void SubsurfaceSPPMRenderer::executePathTracing(const Scene& scene, const Camera& camera, RandomSeq& rseq, HitpointInfo* hp, const int bounceLimit) {
        if (hp->imageX < 0 || hp->imageY < 0 || hp->imageX >= camera.imageW() || hp->imageY >= camera.imageH()) {
            return;
        }

        CameraSample camSample = camera.sample(hp->imageX, hp->imageY, rseq);
        Ray ray = camSample.generateRay();
        const double coeff = camera.sensitivity() / camSample.totalPdf();

        Intersection isect;
        Color weight(1.0, 1.0, 1.0);
        Color accumEmit(0.0, 0.0, 0.0);
        for (int bounce = 0; ; bounce++) {
            double randnum = rseq.next();

            if (!scene.intersect(ray, isect) || bounce > bounceLimit) {
                hp->weight = weight;
                hp->coeff = coeff;
                hp->emission += accumEmit + weight.multiply(scene.envmap().sampleFromDir(ray.direction()));
                break;
            }

            int objectID = isect.objectId();
            const Hitpoint& hitpoint = isect.hitpoint();
            const Material& mtrl = scene.getMaterial(objectID);
            const Vector3D orientNormal = Vector3D::dot(hitpoint.normal(), ray.direction()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

            if (mtrl.reftype == REFLECTION_DIFFUSE) {
                // Ray hits diffuse object, return current weight
                weight = weight.multiply(mtrl.color);
                hp->setPosition(hitpoint.position());
                hp->normal = hitpoint.normal();
                hp->weight = weight;
                hp->coeff = coeff;
                hp->emission += weight.multiply(mtrl.emission) + accumEmit;
                break;
            } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                Vector3D nextDir = Vector3D::reflect(ray.direction(), orientNormal);
                ray = Ray(hitpoint.position(), nextDir);
                weight = weight.multiply(mtrl.color);
            } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                bool isIncoming = Vector3D::dot(hitpoint.normal(), orientNormal) > 0.0;
                Vector3D reflectDir, transmitDir;
                double fresnelRe, fresnelTr;
                bool isTotRef = helper::isTotalRef(isIncoming,
                                                   hitpoint.position(),
                                                   ray.direction(),
                                                   hitpoint.normal(),
                                                   orientNormal,
                                                   &reflectDir,
                                                   &transmitDir,
                                                   &fresnelRe,
                                                   &fresnelTr);

                if (isTotRef) {
                    // Total reflection
                    ray = Ray(hitpoint.position(), reflectDir);
                    weight = weight.multiply(mtrl.color);
                } else {
                    // Trace either reflection or refraction ray with probability
                    const double probability = 0.25 + kReflectProbability * fresnelRe;
                    if (randnum < probability) {
                        // Reflection
                        ray = Ray(hitpoint.position(), reflectDir);
                        weight = weight.multiply(mtrl.color) * (fresnelRe / probability);
                    } else {
                        // Transmit
                        ray = Ray(hitpoint.position(), transmitDir); 
                        weight = weight.multiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                    }
                }
            } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
                const bool isIncoming = hitpoint.normal().dot(orientNormal) > 0.0;

                Vector3D reflectDir, transmitDir;
                double fresnelRe, fresnelTr;
                bool isTotRef = helper::isTotalRef(isIncoming,
                                                    hitpoint.position(),
                                                    ray.direction(),
                                                    hitpoint.normal(),
                                                    orientNormal,
                                                    &reflectDir,
                                                    &transmitDir,
                                                    &fresnelRe,
                                                    &fresnelTr);

                if (isTotRef) {
                    // Total reflection
                    ray = Ray(hitpoint.position(), reflectDir);
                    weight = weight.multiply(mtrl.color);
                } else {
                    // Both reflect and transmit
                    ray = Ray(hitpoint.position(), reflectDir);
                    Color weightTr = Color(weight.multiply(mtrl.color) * fresnelTr);
                    Color irad = integrator.irradiance(hitpoint.position());
                    accumEmit += irad.multiply(weightTr);
                    weight = weight.multiply(mtrl.color) * fresnelRe;                    
                }
            }
        }
    }

}  // namespace spica
