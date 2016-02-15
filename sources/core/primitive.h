#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PRIMITIVE_H_
#define _SPICA_PRIMITIVE_H_

#include <memory>

#include "common.h"
#include "forward_decl.h"

namespace spica {

class SPICA_EXPORTS Primitive {
public:
    virtual ~Primitive() {}
    virtual Bound3d worldBound() const = 0;
    virtual bool    intersect(const Ray& ray, SurfaceInteraction* isect) const = 0;
    virtual const   std::shared_ptr<const AreaLight>& areaLight() const = 0;
    virtual const   std::shared_ptr<const Material>&  material()  const = 0;
    virtual void    setScatterFuncs(SurfaceInteraction* intr,
                                    MemoryArena& arena) const = 0;
};  // class Primitive

class SPICA_EXPORTS GeometricPrimitive : public Primitive {
public:
    // Public methods
    GeometricPrimitive(const std::shared_ptr<Shape>& shape,
                       const std::shared_ptr<Material>& material,
                       const std::shared_ptr<AreaLight>& areaLight);

    virtual Bound3d worldBound() const override;
    virtual bool intersect(const Ray& ray, SurfaceInteraction* isect) const override;

    const std::shared_ptr<const AreaLight>& areaLight() const override;
    const std::shared_ptr<const Material>&  material()  const override;
    void setScatterFuncs(SurfaceInteraction* intr,
                         MemoryArena& arena) const override;

private:
    // Private fields
    std::shared_ptr<Shape> shape_;
    std::shared_ptr<Material> material_;
    std::shared_ptr<AreaLight> areaLight_;

};  // class GeometricPrimitive

class SPICA_EXPORTS Aggregate : public Primitive {
public:
    const std::shared_ptr<const AreaLight>& areaLight() const override;
    const std::shared_ptr<const Material>&  material()  const override;
    void  setScatterFuncs(SurfaceInteraction* intr, 
                          MemoryArena& arena) const override;
};  // class Aggregate

}  // namespace spica

#endif  // _SPICA_PRIMITIVE_H_
