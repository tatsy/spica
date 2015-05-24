#define SPICA_PHOTON_MAPPING_EXPORT
#include "photon_mapping.h"

#include <cmath>
#include <cstdio>
#include <algorithm>

#include "renderer_helper.h"
#include "../utils/sampler.h"

namespace spica {

    // --------------------------------------------------
    // Photon map
    // --------------------------------------------------
    Photon::Photon()
        : Vector3()
        , _flux()
        , _direction()
        , _normal()
    {
    }

    Photon::Photon(const Vector3& position, const Color& flux, const Vector3& direction, const Vector3& normal)
        : Vector3(position)
        , _flux(flux)
        , _direction(direction)
        , _normal(normal)
    {
    }

    Photon::Photon(const Photon& photon)
        : Vector3()
        , _flux()
        , _direction()
        , _normal()
    {
        operator=(photon);
    }

    Photon::~Photon()
    {
    }

    Photon& Photon::operator=(const Photon& photon) {
        Vector3::operator=(photon);
        this->_flux      = photon._flux;
        this->_direction = photon._direction;
        this->_normal = photon._normal;
        return *this;
    }

    // --------------------------------------------------
    // Photon map
    // --------------------------------------------------

    PhotonMap::PhotonMap()
        : _kdtree()
    {
    }

    PhotonMap::~PhotonMap()
    {
    }

    void PhotonMap::clear() {
        _kdtree.release();
    }

    void PhotonMap::construct(const std::vector<Photon>& photons) {
        _kdtree.construct(photons);
    }

    void PhotonMap::findKNN(const Photon& query, std::vector<Photon>* photons, const int numTargetPhotons, const double targetRadius) const {
        _kdtree.knnSearch(query, KnnQuery(K_NEAREST | EPSILON_BALL, numTargetPhotons, targetRadius), photons);
    }

    // --------------------------------------------------
    // Photon mapping renderer
    // --------------------------------------------------

    PMRenderer::PMRenderer()
    {
    }

    PMRenderer::PMRenderer(const PMRenderer& renderer)
    {
    }

    PMRenderer::~PMRenderer()
    {
    }

    PMRenderer& PMRenderer::operator=(const PMRenderer& renderer) {
        return *this;
    }

    int PMRenderer::render(const Scene& scene, const Camera& camera, const Random& rng, const int samplePerPixel, const int numTargetPhotons, const double targetRadius) {
        const int width = camera.imageW();
        const int height = camera.imageH();
        Image image(width, height);

        int proc = 0;
        ompfor (int i = 0; i < samplePerPixel; i++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    image.pixel(width - x - 1, y) += executePT(scene, camera, x, y, rng, numTargetPhotons, targetRadius) / samplePerPixel;
                }

                omplock {
                    proc += 1;
                    printf("%6.2f %% processed...\n", 100.0 * proc / (samplePerPixel * height));
                }
            }
        }
        image.saveBMP("photonmap.bmp");
        return 0;
    }

    Color PMRenderer::executePT(const Scene& scene, const Camera& camera, const double pixelX, const double pixelY, const Random& rng, const int numTargetPhotons, const double targetRadius) const {
        Vector3 posOnSensor;
        Vector3 posOnObjplane;
        Vector3 posOnLens;
        double pImage, pLens;
        camera.samplePoints(pixelX, pixelY, rng, posOnSensor, posOnObjplane, posOnLens, pImage, pLens);
        
        Vector3 lens2sensor = posOnSensor - posOnLens;
        const double cosine = Vector3::dot(camera.direction(), lens2sensor.normalized());
        const double weight = cosine * cosine / lens2sensor.squaredNorm();

        const Ray ray(posOnLens, Vector3::normalize(posOnObjplane - posOnLens));
        return radiance(scene, ray, rng, numTargetPhotons, targetRadius, 0) * (weight * camera.sensitivity() / (pImage * pLens));
    }    
    
    Color PMRenderer::radiance(const Scene& scene, const Ray& ray, const Random& rng, const int numTargetPhotons, const double targetRadius, const int depth, const int depthLimit, const int maxDepth) const {
        Intersection isect;
        if (!scene.intersect(ray, isect)) {
            return scene.bgColor();
        }

        const int objID = isect.objectId();
        const Material& mtrl = scene.getMaterial(objID);
        const Hitpoint hitpoint = isect.hitpoint();
        const Vector3 orientNormal = Vector3::dot(ray.direction(), hitpoint.normal()) < 0.0 ? hitpoint.normal() : -hitpoint.normal();

        // Russian roulette
        double roulette = std::max(mtrl.color.red(), std::max(mtrl.color.green(), mtrl.color.blue()));
        if (depth > maxDepth) {
            if (rng.nextReal() > roulette) {
                return mtrl.emission;
            }
        }
        roulette = 1.0;

        if (mtrl.reftype == REFLECTION_DIFFUSE) {
            // Estimate irradiance with photon map
            Photon query = Photon(hitpoint.position(), Color(), ray.direction(), hitpoint.normal());
            std::vector<Photon> photons;
            photonMap.findKNN(query, &photons, numTargetPhotons, targetRadius);

            const int numPhotons = static_cast<int>(photons.size());
            std::vector<double> dists(numPhotons);
            double maxdist = 0.0;
            for (int i = 0; i < numPhotons; i++) {
                double dist = (photons[i] - query).norm();
                dists[i] = dist;
                maxdist = std::max(maxdist, dist);
            }

            // Cone filter
            const double k = 1.1;
            Color totalFlux = Color(0.0, 0.0, 0.0);
            for (int i = 0; i < numPhotons; i++) {
                const double w = 1.0 - (dists[i] / (k * maxdist));
                const Color v = mtrl.color.cwiseMultiply(photons[i].flux()) / PI;
                totalFlux += w * v;
            }
            totalFlux /= (1.0 - 2.0 / (3.0 * k));
            
            if (numPhotons != 0) {
                return mtrl.emission + totalFlux / ((PI * maxdist * maxdist) * roulette); 
            }
        } else if (mtrl.reftype == REFLECTION_SPECULAR) {
            Vector3 nextDir = Vector3::reflect(ray.direction(), hitpoint.normal());
            Ray nextRay = Ray(hitpoint.position(), nextDir);
            Color nextRad = radiance(scene, nextRay, rng, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
            return mtrl.emission + mtrl.color.cwiseMultiply(nextRad) / roulette;
        } else if (mtrl.reftype == REFLECTION_REFRACTION) {
            bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;
            Vector3 reflectDir, refractDir;
            double fresnelRe, fresnelTr;
            if (helper::isTotalRef(isIncoming, hitpoint.position(), ray.direction(), hitpoint.normal(), orientNormal, &reflectDir, &refractDir, &fresnelRe, &fresnelTr)) {
                // Total reflection
                Ray nextRay = Ray(hitpoint.position(), reflectDir);
                Color nextRad = radiance(scene, nextRay, rng, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                return mtrl.emission + mtrl.color.cwiseMultiply(nextRad) / roulette;
            } else {
                // Reflect or reflact
                const double probRef = 0.25 + REFLECT_PROBABLITY * fresnelRe;
                if (rng.nextReal() < probRef) {
                    // Reflect
                    Ray nextRay = Ray(hitpoint.position(), reflectDir);
                    Color nextRad = radiance(scene, nextRay, rng, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                    return mtrl.emission + mtrl.color.cwiseMultiply(nextRad) * (fresnelRe / (probRef * roulette));
                } else {
                    // Refract
                    Ray nextRay = Ray(hitpoint.position(), refractDir);
                    Color nextRad = radiance(scene, nextRay, rng, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                    return mtrl.emission + mtrl.color.cwiseMultiply(nextRad) * (fresnelTr / ((1.0 - probRef) * roulette));
                }
            }
        } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
            bool isIncoming = Vector3::dot(hitpoint.normal(), orientNormal) > 0.0;
            Vector3 reflectDir, refractDir;
            double fresnelRe, fresnelTr;

            bool isTotRef = helper::isTotalRef(isIncoming,
                                               hitpoint.position(),
                                               ray.direction(),
                                               hitpoint.normal(),
                                               orientNormal,
                                               &reflectDir,
                                               &refractDir,
                                               &fresnelRe,
                                               &fresnelTr);
            
            if (isTotRef) {
                // Total reflection
                Ray nextRay(hitpoint.position(), reflectDir);
                Color nextRad = radiance(scene, nextRay, rng, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                return mtrl.emission + mtrl.color.cwiseMultiply(nextRad) / roulette;
            }
            
            const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;
            if (rng.nextReal() < probability) {
                // Reflect
                Ray nextRay = Ray(hitpoint.position(), reflectDir);
                Color nextRad = radiance(scene, nextRay, rng, numTargetPhotons, targetRadius, depth + 1, depthLimit, maxDepth);
                return mtrl.emission + mtrl.color.cwiseMultiply(nextRad) * (fresnelRe / (probability * roulette));
            } else {
                // Transmit, then estimate irradiance with photon map
                Photon query = Photon(hitpoint.position(), Color(), ray.direction(), hitpoint.normal());
                std::vector<Photon> photons;
                photonMap.findKNN(query, &photons, numTargetPhotons, targetRadius);

                const int numPhotons = static_cast<int>(photons.size());
                std::vector<double> dists(numPhotons);
                double maxdist = 0.0;
                for (int i = 0; i < numPhotons; i++) {
                    double dist = (photons[i] - query).norm();
                    dists[i] = dist;
                    maxdist = std::max(maxdist, dist);
                }
        
                // Dipole model
                Color totalFlux = Color(0.0, 0.0, 0.0);
                DiffusionReflectance Rd(0.1, 100.0, 1.3);
                for (int i = 0; i < numPhotons; i++) {
                    Vector3 diff = photons[i] - hitpoint.position();
                    const double w = Rd(diff.squaredNorm());
                    const Color v = mtrl.color.cwiseMultiply(photons[i].flux());
                    totalFlux += w * v / PI;
                }
            
                if (numPhotons != 0) {
                    return mtrl.emission + totalFlux * (fresnelTr / ((1.0 - probability) * roulette)); 
                }
            }
        }

        return Color();
    }

    void PMRenderer::buildPM(const Scene& scene, const Camera& camera, const Random& rng, const int numPhotons) {
        std::cout << "Shooting photons..." << std::endl;

        // Shooting photons
        std::vector<Photon> photons;
        int proc = 0;
        ompfor (int pid = 0; pid < numPhotons; pid++) {
            // Generate sample on the light
            const int lightID = scene.lightID();
            const Primitive* light = scene.get(lightID);

            Vector3 positionOnLignt, normalOnLight;
            sampler::on(light, &positionOnLignt, &normalOnLight);

            Color currentFlux = light->area() * scene.getMaterial(lightID).emission * PI / numPhotons;
            Vector3 nextDir;
            sampler::onHemisphere(normalOnLight, &nextDir);

            Ray currentRay(positionOnLignt, nextDir);
            Vector3 prevNormal = normalOnLight;

            for (;;) {
                // Remove photon with zero flux
                if (std::max(currentFlux.red(), std::max(currentFlux.green(), currentFlux.blue())) < 0.0) {
                    break;
                }

                Intersection isect;
                bool isHit = scene.intersect(currentRay, isect);

                // If not hit the scene, then break
                if (!isHit) {
                    break;
                }

                // Hitting object
                const Material& mtrl = scene.getMaterial(isect.objectId());
                const Hitpoint& hitpoint = isect.hitpoint();

                Vector3 orientingNormal = hitpoint.normal();
                if (Vector3::dot(hitpoint.normal(), currentRay.direction()) > 0.0) {
                    orientingNormal *= -1.0;
                }

                if (mtrl.reftype == REFLECTION_DIFFUSE) {
                    omplock {
                        photons.push_back(Photon(hitpoint.position(), currentFlux, currentRay.direction(), hitpoint.normal()));
                    }

                    const double probContinueTrace = (mtrl.color.red() + mtrl.color.green() + mtrl.color.blue()) / 3.0;
                    if (probContinueTrace > rng.nextReal()) {
                        // Continue trace
                        sampler::onHemisphere(orientingNormal, &nextDir);
                        currentRay = Ray(hitpoint.position(), nextDir);
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color) / probContinueTrace;
                    } else {
                        // Absorb (finish trace)
                        break;
                    }
                } else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nextDir = Vector3::reflect(currentRay.direction(), hitpoint.normal());
                    currentRay = Ray(hitpoint.position(), nextDir);
                } else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    bool isIncoming = Vector3::dot(hitpoint.normal(), orientingNormal) > 0.0;

                    Vector3 reflectDir, transmitDir;
                    double fresnelRe, fresnelTr;
                    bool isTotRef = helper::isTotalRef(isIncoming,
                                                       hitpoint.position(),
                                                       currentRay.direction(),
                                                       hitpoint.normal(),
                                                       orientingNormal,
                                                       &reflectDir,
                                                       &transmitDir,
                                                       &fresnelRe,
                                                       &fresnelTr);


                    Ray reflectRay = Ray(hitpoint.position(), reflectDir);

                    if (isTotRef) {
                        // Total reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color);
                        continue;
                    }

                    const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;

                    if (rng.nextReal() < probability) {
                        // Reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color) * (fresnelRe / probability);
                    } else {
                        // Reflaction
                        currentRay = Ray(hitpoint.position(), transmitDir);
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                    }
                } else if (mtrl.reftype == REFLECTION_SUBSURFACE) {
                    bool isIncoming = Vector3::dot(hitpoint.normal(), orientingNormal) > 0.0;

                    Vector3 reflectDir, transmitDir;
                    double fresnelRe, fresnelTr;
                    bool isTotRef = helper::isTotalRef(isIncoming,
                                                       hitpoint.position(),
                                                       currentRay.direction(),
                                                       hitpoint.normal(),
                                                       orientingNormal,
                                                       &reflectDir,
                                                       &transmitDir,
                                                       &fresnelRe,
                                                       &fresnelTr);

                    Ray reflectRay(hitpoint.position(), reflectDir);
                    if (isTotRef) {
                        // Total reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color);
                        continue;
                    }

                    const double probability = 0.25 + REFLECT_PROBABLITY * fresnelRe;

                    if (rng.nextReal() < probability) {
                        // Reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color) * (fresnelRe / probability);
                    } else {
                        // Transmit (Absorption)
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color) * (fresnelTr / (1.0 - probability));
                        omplock {
                            photons.push_back(Photon(hitpoint.position(), currentFlux, currentRay.direction(), hitpoint.normal()));
                        }
                        break;
                    }
                }
            }

            omplock {
                proc++;
                if (proc % 1000 == 0) {
                    printf("%6.2f %% processed...\r", 100.0 * proc / numPhotons);
                }
            }
        }
        printf("\n\n");

        // Construct photon map
        photonMap.clear();
        photonMap.construct(photons);
    }

}

