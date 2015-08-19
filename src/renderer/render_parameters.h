#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_RENDER_PARAMETERS_H_
#define _SPICA_RENDER_PARAMETERS_H_

#include "renderer_constants.h"

namespace spica {

    class RenderParameters {
    private:
        int        _bounceLimit;
        int        _bounceStartRoulette;
        int        _samplePerPixel;
        int        _castPhotons;
        int        _gatherPhotons;
        double     _gatherRadius;
        RandomType _randomType;

    public:
        RenderParameters(int samplePerPixel = 8,
                         int bounceLimit = 32,
                         int bounceStartRoulette = 5,
                         int castPhotons = 1000000,
                         int gatherPhotons = 64,
                         double gatherRadius = 32.0,
                         RandomType randomType = PSEUDO_RANDOM_TWISTER)
            : _samplePerPixel(samplePerPixel)
            , _bounceLimit(bounceLimit)
            , _bounceStartRoulette(bounceStartRoulette)
            , _castPhotons(castPhotons)
            , _gatherPhotons(gatherPhotons)
            , _gatherRadius(gatherRadius)
            , _randomType(randomType)
        {
        }

        RenderParameters(const RenderParameters& params)
            : _samplePerPixel()
            , _bounceLimit()
            , _bounceStartRoulette()
            , _castPhotons()
            , _gatherPhotons()
            , _gatherRadius()
            , _randomType()
        {
            this->operator=(params);
        }

        ~RenderParameters()
        {
        }

        RenderParameters& operator=(const RenderParameters& params) {
            this->_samplePerPixel      = params._samplePerPixel;
            this->_bounceLimit         = params._bounceLimit;
            this->_bounceStartRoulette = params._bounceStartRoulette;
            this->_castPhotons         = params._castPhotons;
            this->_gatherPhotons       = params._gatherPhotons;
            this->_gatherRadius        = params._gatherRadius;
            this->_randomType          = params._randomType;
            return *this;
        }

        // Maximum bounces
        inline int  bounceLimit() const     { return _bounceLimit;   }  
        inline void bounceLimit(int newval) { _bounceLimit = newval; }

        // # of bounces that the Russian roulette starts
        inline int  bounceStartRoulette() const     { return _bounceStartRoulette;   }
        inline void bounceStartRoulette(int newval) { _bounceStartRoulette = newval; }

        // # of samples for a pixel
        inline int  samplePerPixel() const     { return _samplePerPixel;   }
        inline void samplePerPixel(int newval) { _samplePerPixel = newval; }

        // # of photons casted to construct a photon map
        inline int  castPhotons() const     { return _castPhotons;   }
        inline void castPhotons(int newval) { _castPhotons = newval; }

        // # of photons used to evaluate the radiance for a hitpoint
        inline int  gatherPhotons() const     { return _gatherPhotons;   }
        inline void gatherPhotons(int newval) { _gatherPhotons = newval; }

        // Search radius to collect kNN photons
        inline double gatherRadius() const        { return _gatherRadius;   }
        inline void   gatherRadius(double newval) { _gatherRadius = newval; }

        // Type of random number sampler
        inline RandomType randomType() const                { return _randomType;       }
        inline void       randomType(RandomType randomType) { _randomType = randomType; }
    };

}  // namespace spica

#endif  // _SPICA_RENDER_PARAMETERS_H_
