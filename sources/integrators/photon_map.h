#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PHOTON_MAP_H_
#define _SPICA_PHOTON_MAP_H_

#include "core/common.h"

#include "core/spectrum.h"
#include "core/kdtree.h"
#include "core/uncopyable.h"
#include "core/stack.h"

#include "core/vector3d.h"
#include "core/point3d.h"
#include "core/normal3d.h"
#include "core/random.h"

namespace spica {

// ------------------------------------------------------------------------
// Photon
// ------------------------------------------------------------------------
class SPICA_EXPORTS Photon  {
public:
    Photon();
    Photon(const Point3d& pos, const Spectrum& beta, 
           const Vector3d& wi, const Normal3d& normal);
    Photon(const Photon& photon);
    ~Photon();

    Photon& operator=(const Photon& photon);
    double operator[](int i) const;

    static double distance(const Photon& p1, const Photon& p2);

    inline Point3d  pos()    const { return pos_; }
    inline Spectrum beta()   const { return beta_; }
    inline Vector3d wi()     const { return wi_; }
    inline Normal3d normal() const { return normal_; }
    inline const Material* const material() const { return material_; }
    
private:
    Point3d  pos_;
    Spectrum beta_;
    Vector3d wi_;
    Normal3d normal_;
    const Material* material_;
};

// ------------------------------------------------------------------------
// Photon map
// ------------------------------------------------------------------------
class SPICA_EXPORTS PhotonMap : public Uncopyable {
public:
    // Public methods
    PhotonMap();
    virtual ~PhotonMap();

    void clear();
    virtual void construct(const Scene& scene,
                           RenderParams& params,
                           Sampler &sampler);

    /**
     * Evaluate radiance at the point and to the direction.
     */
    virtual Spectrum evaluateL(const SurfaceInteraction& po,
                               int gatherPhotons, double gatherRadius) const;

    virtual Spectrum evaluateL(const MediumInteraction& mi,
                               int gatherPhotons, double gatherRadius) const;
        
protected:
    // Protected methods
    void knnFind(const Photon& photon, std::vector<Photon>* photons, 
                    int gatherPhotons, double gatherRadius) const;

    void tracePhoton(const Scene& scene,
                     RenderParams& params,
                     const Ray& r,
                     const Spectrum& b,
                     Sampler& sampler,
                     MemoryArena& arena,
                     std::vector<Photon>* photons);
    
    // Protected fields
    KdTree<Photon> _kdtree;    
};

}  // namespace spica

#endif  // _SPICA_PHOTON_MAP_H_
