#ifndef _SPICA_PHOTON_MAP_H_
#define _SPICA_PHOTON_MAP_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_PHOTON_MAP_EXPORT
        #define SPICA_PHOTON_MAP_DLL __declspec(dllexport)
    #else
        #define SPICA_PHOTON_MAP_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_PHOTON_MAP_DLL
#endif

#include "../utils/vector3.h"
#include "../utils/color.h"
#include "../utils/kdtree.h"
#include "../utils/uncopyable.h"

#include "../random/random.h"

namespace spica {

    class Scene;

    class SPICA_PHOTON_MAP_DLL Photon : public Vector3 {
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

        static Photon sample(const Scene& scene, RandomSeq& rseq, const int numPhotons);

        inline Color   flux()      const { return _flux; }
        inline Vector3 direction() const { return _direction; }
        inline Vector3 normal()    const { return _normal; }
    };

    class SPICA_PHOTON_MAP_DLL PhotonMap : public Uncopyable {
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
