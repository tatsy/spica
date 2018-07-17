#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MEDIUM_H_
#define _SPICA_MEDIUM_H_

#include <memory>

#include "core/core.hpp"
#include "core/common.h"
#include "core/cobject.h"
#include "core/spectrum.h"

#include "core/render.hpp"

namespace spica {

class SPICA_EXPORTS Medium : public CObject {
public:
    virtual Spectrum Tr(const Ray& ray, Sampler& sampler) const = 0;
    virtual Spectrum sample(const Ray& ray, Sampler& sampler,
                            MemoryArena& arena, MediumInteraction* mi) const = 0;
    virtual Spectrum sigmaExt(const Point3d &pos) const = 0;
};  // class Medium

class MediumInterface {
public:
    MediumInterface()
        : inside_{ nullptr }
        , outside_{ nullptr } {
    }

    explicit MediumInterface(const Medium *medium)
        : inside_{ medium }
        , outside_{ medium } {
    }

    MediumInterface(const Medium *inside,
                    const Medium *outside)
        : inside_{ inside }
        , outside_{ outside } {
    }

    bool isMediumTransition() const { return inside_ != outside_; }

    inline const Medium* inside() const { return inside_; }
    inline const Medium* outside() const { return outside_; }

private:
    const Medium *inside_;
    const Medium *outside_;

};  // class MediumInterface

}  // namespace spica

#endif  // _SPICA_MEDIUM_H_
