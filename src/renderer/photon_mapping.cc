#define SPICA_PHOTON_MAPPING_EXPORT
#include "photon_mapping.h"

#include <algorithm>

#include "../utils/sampler.h"

namespace spica {

    // --------------------------------------------------
    // Photon map
    // --------------------------------------------------
    Photon::Photon()
        : Vector3()
        , _flux()
        , _direction()
    {
    }

    Photon::Photon(const Vector3& position, const Color& flux, const Vector3& direction)
        : Vector3(position)
        , _flux(flux)
        , _direction(direction)
    {
    }

    Photon::Photon(const Photon& photon)
        : Vector3()
        , _flux()
        , _direction()
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

    int PMRenderer::render(const Scene& scene, const Camera& camera, const Random& rng) {
        return 0;
    }

    void PMRenderer::buildPM(const Scene& scene, const Camera& camera, const Random& rng, const int numPhotons) {
        std::cout << "Shooting photons..." << std::endl;

        std::vector<Photon> photons;
        for (int pid = 0; pid < numPhotons; pid++) {

            // Generate sample on the light
            const int lightID = scene.lightID();
            const Primitive* light = scene.get(lightID);

            Vector3 positionOnLignt, normalOnLight;
            sampler::on(light, &positionOnLignt, &normalOnLight);
            double pdfAreaOnLight = 1.0 / light->area();

            double totalPdfA = pdfAreaOnLight;

            Color currentFlux = light->area() * scene.getMaterial(lightID).emission / numPhotons;
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
                    photons.push_back(Photon(hitpoint.position(), currentFlux, currentRay.direction()));

                    const double probContinueTrace = (mtrl.color.red() + mtrl.color.green() + mtrl.color.blue()) / 3.0;
                    if (probContinueTrace > rng.randReal()) {
                        // Continue trace
                        sampler::onHemisphere(orientingNormal, &nextDir);
                        currentRay = Ray(hitpoint.position(), nextDir);
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color) / probContinueTrace;
                    }
                    else {
                        // Absorb (finish trace)
                        break;
                    }
                }
                else if (mtrl.reftype == REFLECTION_SPECULAR) {
                    nextDir = Vector3::reflect(currentRay.direction(), hitpoint.normal());
                    currentRay = Ray(hitpoint.position(), nextDir);
                }
                else if (mtrl.reftype == REFLECTION_REFRACTION) {
                    // Ray of reflection
                    Vector3 reflectDir = Vector3::reflect(currentRay.direction(), hitpoint.normal());
                    Ray reflectRay = Ray(hitpoint.position(), reflectDir);

                    // Determine reflact or not
                    bool isIncoming = Vector3::dot(hitpoint.normal(), orientingNormal) > 0.0;
                    const double nnt = isIncoming ? IOR_VACCUM / IOR_OBJECT : IOR_OBJECT / IOR_VACCUM;
                    const double ddn = Vector3::dot(currentRay.direction(), orientingNormal);
                    const double cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

                    if (cos2t < 0.0) {
                        // Total reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color);
                        continue;
                    }

                    // Ray of reflaction
                    Vector3 reflactDir = Vector3::normalize(currentRay.direction() * nnt - ((isIncoming ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))) * hitpoint.normal());

                    // Compute reflaction ratio
                    const double a = IOR_VACCUM - IOR_OBJECT;
                    const double b = IOR_VACCUM + IOR_OBJECT;
                    const double R0 = (a * a) / (b * b);
                    const double c = 1.0 - (isIncoming ? -ddn : Vector3::dot(reflactDir, hitpoint.normal()));
                    const double Re = R0 + (1.0 - R0) * pow(c, 5.0);
                    const double Tr = 1.0 - Re;

                    if (rng.randReal() < Re) {
                        // Reflection
                        currentRay = reflectRay;
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color);
                    }
                    else {
                        // Reflaction
                        currentRay = Ray(hitpoint.position(), reflactDir);
                        currentFlux = currentFlux.cwiseMultiply(mtrl.color);
                    }
                }
            }
        }

        // Construct photon map
        photonMap.clear();
        photonMap.construct(photons);
    }

}

