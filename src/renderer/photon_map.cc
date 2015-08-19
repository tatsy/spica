#define SPICA_PHOTON_MAP_EXPORT
#include "photon_map.h"

#include "../utils/sampler.h"
#include "scene.h"

namespace spica {

    Photon::Photon()
        : _position()
        , _flux()
        , _direction()
        , _normal()
    {
    }

    Photon::Photon(const Vector3D& position, const Color& flux, const Vector3D& direction, const Vector3D& normal)
        : _position(position)
        , _flux(flux)
        , _direction(direction)
        , _normal(normal)
    {
    }

    Photon::Photon(const Photon& photon)
        : _position()
        , _flux()
        , _direction()
        , _normal()
    {
        this->operator=(photon);
    }

    Photon::~Photon()
    {
    }

    Photon& Photon::operator=(const Photon& photon) {
        this->_position  = photon._position;
        this->_flux      = photon._flux;
        this->_direction = photon._direction;
        this->_normal    = photon._normal;
        return *this;
    }

    double Photon::get(int id) const {
        return _position.get(id);
    }

    double Photon::distance(const Photon& p1, const Photon& p2) {
        return (p1._position - p2._position).norm();
    }

    Photon Photon::sample(const Scene& scene, Stack<double>& rstk, const int numPhotons) {
        /*
        const int lightID = scene.lightID();
        if (lightID >= 0) {
            const IGeometry* light = scene.get(lightID);

            const double r1 = rseq.next();
            const double r2 = rseq.next();

            Vector3D posLight, normalLight;
            sampler::on(light, &posLight, &normalLight, r1, r2);
            Color currentFlux = Color(light->area() * scene.getMaterial(lightID).emission * PI / numPhotons);
            return Photon(posLight, currentFlux, normalLight, normalLight);
        } else {
            return scene.envmap().samplePhoton(rseq, numPhotons);
        }
        */
        Assertion(false, "This is not implemented!!");
        return Photon();
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