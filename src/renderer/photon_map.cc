#define SPICA_PHOTON_MAP_EXPORT
#include "photon_map.h"

#include "../utils/sampler.h"
#include "scene.h"

namespace spica {

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

    Photon Photon::sample(const Scene& scene, RandomSeq& rseq, const int numPhotons) {
        const int lightID = scene.lightID();
        if (lightID >= 0) {
            const Primitive* light = scene.get(lightID);

            const double r1 = rseq.next();
            const double r2 = rseq.next();

            Vector3 posLight, normalLight;
            sampler::on(light, &posLight, &normalLight, r1, r2);
            Color currentFlux = Color(light->area() * scene.getMaterial(lightID).emission * PI / numPhotons);
            return Photon(posLight, currentFlux, normalLight, normalLight);
        } else {
            return scene.envmap().samplePhoton(rseq, numPhotons);
        }
    }

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

}