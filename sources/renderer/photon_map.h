#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHOTON_MAP_H_
#define _SPICA_PHOTON_MAP_H_

#include "../bsdf/bsdf.h"

#include "../core/color.h"
#include "../core/kdtree.h"
#include "../core/uncopyable.h"
#include "../core/stack.h"

#include "../math/vector3d.h"
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
    private:
        Vector3D _position;
        Color    _flux;
        Vector3D _direction;
        Vector3D _normal;

    public:
        Photon();
        Photon(const Vector3D& position, const Color& flux, 
               const Vector3D& direction, const Vector3D& normal);
        Photon(const Photon& photon);
        ~Photon();

        Photon& operator=(const Photon& photon);

        inline double get(int id) const;

        static double distance(const Photon& p1, const Photon& p2);

        inline Vector3D position()  const { return _position;  }
        inline Color    flux()      const { return _flux;      }
        inline Vector3D direction() const { return _direction; }
        inline Vector3D normal()    const { return _normal;    }
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

        Color evaluate(const Vector3D& position,
                       const Vector3D& normal,
                       int gatherPhotons, double gatherRadius) const;
        
    private:
        void knnFind(const Photon& photon, std::vector<Photon>* photons, 
                     int gatherPhotons, double gatherRadius) const;

        void tracePhoton(const Scene& scene, const Ray& ray,
                         const RenderParameters& params,
                         const Color& flux, Stack<double>& rstk,
                         int bounces, BsdfType absorbBsdf,
                         std::vector<Photon>* photons);
    };

}

#endif
