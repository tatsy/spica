#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHOTON_MAP_H_
#define _SPICA_PHOTON_MAP_H_

#include "../bsdf/bsdf.h"

#include "../core/spectrum.h"
#include "../core/kdtree.h"
#include "../core/uncopyable.h"
#include "../core/stack.h"

#include "../math/vector3d.h"
#include "../core/point3d.h"
#include "../core/normal3d.h"
#include "../random/random.h"

namespace spica {

    // Forward declarations
    class Ray;
    class Scene;
    class Camera;
    class RenderParameters;

    // ------------------------------------------------------------------------
    // Photon
    // ------------------------------------------------------------------------
    class SPICA_EXPORTS Photon  {
    public:
        Photon();
        Photon(const Point& position, const Spectrum& flux, 
               const Vector3D& direction, const Normal& normal);
        Photon(const Photon& photon);
        ~Photon();

        Photon& operator=(const Photon& photon);

        inline double get(int id) const;
        static double distance(const Photon& p1, const Photon& p2);

        inline Point    position()  const { return _position;  }
        inline Spectrum flux()      const { return _flux;      }
        inline Vector3D direction() const { return _direction; }
        inline Normal   normal()    const { return _normal;    }

    private:
        Point _position;
        Spectrum _flux;
        Vector3D _direction;
        Normal _normal;
    };

    // ------------------------------------------------------------------------
    // Photon map
    // ------------------------------------------------------------------------
    class SPICA_EXPORTS PhotonMap : public Uncopyable {
    private:
        KdTree<Photon> _kdtree;

    public:
        PhotonMap();
        ~PhotonMap();

        void clear();
        void construct(const Scene& scene,
                       const RenderParameters& params,
                       BsdfType absorbBsdf);

        Spectrum evaluate(const Point& position,
                          const Normal& normal,
                          int gatherPhotons, double gatherRadius) const;
        
    private:
        void knnFind(const Photon& photon, std::vector<Photon>* photons, 
                     int gatherPhotons, double gatherRadius) const;

        void tracePhoton(const Scene& scene, const Ray& ray,
                         const RenderParameters& params,
                         const Spectrum& flux, Stack<double>& rstk,
                         int bounces, BsdfType absorbBsdf,
                         std::vector<Photon>* photons);
    };

}

#endif
