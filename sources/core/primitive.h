#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PRIMITIVE_H_
#define _SPICA_PRIMITIVE_H_

#include <memory>
#include <vector>

#include "common.h"
#include "forward_decl.h"

namespace spica {

class SPICA_EXPORTS Primitive {
public:
    virtual ~Primitive() {}
    virtual Bounds3d worldBound() const = 0;
    virtual bool    intersect(Ray& ray, SurfaceInteraction* isect) const = 0;
    virtual bool    intersect(Ray& ray) const = 0;
    virtual const   AreaLight* areaLight() const = 0;
    virtual const   Material*  material()  const = 0;
    virtual std::vector<Triangle> triangulate() const = 0;
    virtual void    setScatterFuncs(SurfaceInteraction* intr,
                                    MemoryArena& arena) const = 0;
};  // class Primitive

class SPICA_EXPORTS GeometricPrimitive : public Primitive {
public:
    // Public methods
    GeometricPrimitive(const std::shared_ptr<Shape>& shape,
                       const std::shared_ptr<Material>& material,
                       const std::shared_ptr<AreaLight>& areaLight,
                       const std::shared_ptr<MediumInterface>& mediumInterface = nullptr);

    virtual Bounds3d worldBound() const override;
    virtual bool intersect(Ray& ray, SurfaceInteraction* isect) const override;
    virtual bool intersect(Ray& ray) const override;

    const AreaLight* areaLight() const override;
    const Material*  material()  const override;
    std::vector<Triangle> triangulate() const override;
    void setScatterFuncs(SurfaceInteraction* intr,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Shape>     shape_ = nullptr;
    std::shared_ptr<Material>  material_ = nullptr;
    std::shared_ptr<AreaLight> areaLight_ = nullptr;
    std::shared_ptr<MediumInterface> mediumInterface_ = nullptr;

};  // class GeometricPrimitive

class SPICA_EXPORTS Aggregate : public Primitive {
public:
    const AreaLight* areaLight() const override;
    const Material*  material()  const override;
    void  setScatterFuncs(SurfaceInteraction* intr, 
                          MemoryArena& arena) const override;
};  // class Aggregate

}  // namespace spica

#endif  // _SPICA_PRIMITIVE_H_
