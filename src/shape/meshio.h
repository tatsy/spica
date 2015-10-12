#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_MESHIO_H_
#define _SPICA_MESHIO_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_MESHIO_EXPORT
        #define SPICA_MESHIO_DLL __declspec(dllexport)
    #else
        #define SPICA_MESHIO_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_MESHIO_DLL
#endif

#include <string>

#include "../core/forward_decl.h"
#include "../core/uncopyable.h"

namespace spica {

    /** Mesh IO interface
     */
    class MeshIO : private Uncopyable {
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
    class SPICA_MESHIO_DLL PLYMeshIO : public MeshIO {
    public:
        PLYMeshIO();
        ~PLYMeshIO();

        void load(const std::string& filename, Trimesh* trimesh) const override;
        void save(const std::string& filename,
                  const Trimesh& trimesh) const override;
    };

    /** Mesh IO for OBJ format.
     */
    class SPICA_MESHIO_DLL OBJMeshIO : public MeshIO {
    public:
        OBJMeshIO();
        ~OBJMeshIO();

        void load(const std::string& filename, Trimesh* trimesh) const override;
        void save(const std::string& filename,
                  const Trimesh& trimesh) const override;
    };

}  // namespace spica

#endif  // _SPICA_MESHIO_H_
