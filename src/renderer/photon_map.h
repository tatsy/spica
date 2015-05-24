#ifndef _SPICA_PHOTON_MAP_H_
#define _SPICA_PHOTON_MAP_H_

#include "../utils/vector3.h"
#include "../utils/color.h"
#include "../utils/kdtree.h"
#include "../utils/uncopyable.h"

namespace spica {

    class Photon : public Vector3 {
    private:
        Color   _flux;
        Vector3 _direction;
        Vector3 _normal;

    public:
        Photon();
        Photon(const Vector3& position, const Color& flux, const Vector3& direction, const Vector3& normal);
        Photon(const Photon& photon);
        ~Photon();

        Photon& operator=(const Photon& photon);

        inline Color   flux()      const { return _flux; }
        inline Vector3 direction() const { return _direction; }
        inline Vector3 normal()    const { return _normal; }
    };

    class PhotonMap : public Uncopyable {
    private:
        KdTree<Photon> _kdtree;

    public:
        PhotonMap();
        ~PhotonMap();

        void clear();
        void construct(const std::vector<Photon>& photons);

        void findKNN(const Photon& photon, std::vector<Photon>* photons, const int numTargetPhotons, const double targetRadius) const;
    };

}

#endif
