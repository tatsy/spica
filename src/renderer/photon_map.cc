#include "photon_map.h"

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