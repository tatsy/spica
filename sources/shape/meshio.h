#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MESHIO_H_
#define _SPICA_MESHIO_H_

#include <string>
#include <vector>
#include <memory>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/uncopyable.h"
#include "../math/transform.h"

namespace spica {

/**
 * Shape with materials.
 */
class SPICA_EXPORTS ShapeGroup {
public:
    ShapeGroup();
    explicit ShapeGroup(
        const std::vector<std::shared_ptr<Shape>>& shapes,
        const std::shared_ptr<Texture<Spectrum>>& mapKd = nullptr,
        const std::shared_ptr<Texture<double>> & bumpMap = nullptr);

    ~ShapeGroup();

    ShapeGroup(const ShapeGroup& sg);
    ShapeGroup(ShapeGroup&& sg);

    ShapeGroup& operator=(const ShapeGroup& sg);
    ShapeGroup& operator=(ShapeGroup&& sg);

    inline const std::vector<std::shared_ptr<Shape>>& shapes() const {
        return shapes_;
    }

    inline const std::shared_ptr<Texture<Spectrum>>& mapKd() const {
        return mapKd_;
    }

    inline const std::shared_ptr<Texture<double>>& bumpMap() const {
        return bumpMap_;
    }

private:
    std::vector<std::shared_ptr<Shape>> shapes_;
    std::shared_ptr<Texture<Spectrum>> mapKd_ = nullptr;
    std::shared_ptr<Texture<double>> bumpMap_ = nullptr;
};

/**
 * Mesh IO interface.
 */
class SPICA_EXPORTS MeshIO : private Uncopyable {
public:
    MeshIO();
    virtual ~MeshIO();

    /**
     * Load mesh from .obj or .ply file.
     */
    std::vector<ShapeGroup> load(const std::string& filename,
                                 const Transform& o2w = Transform()) const;

    void save(const std::string& filename,
              const std::vector<Triangle>& trimesh) const;

private:
    std::vector<ShapeGroup> loadObj(const std::string& filename,
                                    const Transform& o2w = Transform()) const;

    std::vector<ShapeGroup> loadPly(const std::string& filename,
                                    const Transform& o2w = Transform()) const;
};

}  // namespace spica

#endif  // _SPICA_MESHIO_H_
