#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MESHIO_H_
#define _SPICA_MESHIO_H_

#include <string>

#include "../core/common.h"
#include "../core/forward_decl.h"
#include "../core/uncopyable.h"

namespace spica {

    /** Mesh IO interface
     */
    class SPICA_EXPORTS MeshIO : private Uncopyable {
    public:
        MeshIO() {}
        virtual ~MeshIO() {}

        virtual void load(const std::string& filename,
                          Trimesh* trimesh) const = 0;
        virtual void save(const std::string& filename,
                          const Trimesh& trimesh) const = 0;
    };

    /** Mesh IO for PLY format.
     */
    class SPICA_EXPORTS PLYMeshIO : public MeshIO {
    public:
        PLYMeshIO();
        ~PLYMeshIO();

        void load(const std::string& filename, Trimesh* trimesh) const override;
        void save(const std::string& filename,
                  const Trimesh& trimesh) const override;
    };

    /** Mesh IO for OBJ format.
     */
    class SPICA_EXPORTS OBJMeshIO : public MeshIO {
    public:
        OBJMeshIO();
        ~OBJMeshIO();

        void load(const std::string& filename, Trimesh* trimesh) const override;
        void save(const std::string& filename,
                  const Trimesh& trimesh) const override;

    private:
        static Image getTexture(const std::string& filename);
    };

}  // namespace spica

#endif  // _SPICA_MESHIO_H_
