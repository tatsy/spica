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
 * Mesh IO interface.
 */
class SPICA_EXPORTS MeshIO : private Uncopyable {
public:
    MeshIO() {}
    virtual ~MeshIO() {}

    virtual std::vector<std::shared_ptr<Shape>>
        load(const std::string& filename, const Transform& o2w = Transform()) const = 0;
    virtual void save(const std::string& filename,
                      const std::vector<Triangle>& trimesh) const = 0;
};

/** Mesh IO for PLY format.
 */
class SPICA_EXPORTS PLYMeshIO : public MeshIO {
public:
    PLYMeshIO();
    ~PLYMeshIO();

    std::vector<std::shared_ptr<Shape>>
        load(const std::string& filename, const Transform& o2w = Transform()) const override;
    void save(const std::string& filename, 
              const std::vector<Triangle>& trimesh) const override;
};

/** Mesh IO for OBJ format.
 */
class SPICA_EXPORTS OBJMeshIO : public MeshIO {
public:
    OBJMeshIO();
    ~OBJMeshIO();

    std::vector<std::shared_ptr<Shape>>
        load(const std::string& filename, const Transform& o2w = Transform()) const override;
    void save(const std::string& filename,
              const std::vector<Triangle>& trimesh) const override;

private:
    static Image getTexture(const std::string& filename);
};

}  // namespace spica

#endif  // _SPICA_MESHIO_H_
