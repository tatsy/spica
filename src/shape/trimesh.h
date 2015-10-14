#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_TRIMESH_H_
#define _SPICA_TRIMESH_H_

#include <string>
#include <memory>
#include <vector>
#include <tuple>

#include "../core/forward_decl.h"
#include "../core/triplet.h"
#include "../shape/shape_interface.h"
#include "../scenes/vertex_data.h"
#include "../accel/accel.h"

namespace spica {
    
    /** Triangle mesh class
     *  @ingroup shape_module
     */
    class SPICA_EXPORTS Trimesh : public IShape {
    private:
        std::vector<VertexData> _vertices;
        std::vector<Triplet>    _faces;
        Accelerator _accel;
        std::shared_ptr<Image> _texture;

    public:
        /** Contructor.
         */
        Trimesh();

        /** Constructor.
         *  @param filename: Mesh file of PLY or OBJ format.
         */
        explicit Trimesh(const std::string& filename);

        /** Constructor.
         *  @param vertices: Vertices to form trimesh
         *  @param faceIDs: Face IDs (stating from 0)
         */
        Trimesh(const std::vector<Vector3D>& vertices,
                const std::vector<Triplet>& faceIDs);

        /** Constructor.
         *  @param vertices: Vertices to form trimesh
         *  @param texcoords: Texture coordinates.
         *  @param faceIDs: Face IDs (stating from 0)
         */
        Trimesh(const std::vector<Vector3D>& vertices,
                const std::vector<Vector2D>& texcoords,
                const std::vector<Triplet>& faceIDs);

        /** The Trimesh constructor (copy).
         */
        Trimesh(const Trimesh& trimesh);

        /** The Trimesh constructor (move).
         */
        Trimesh(Trimesh&& trimesh);

        /** The Trimesh destructor.
         */
        virtual ~Trimesh();

        /** Assignment operator (copy).
         */
        Trimesh& operator=(const Trimesh& trimesh);

        /** Assignment operator (move).
         */
        Trimesh& operator=(Trimesh&& trimesh);

        /** Intersection test.
         * @param[in]  ray: input to check intersect with
         * @param[out] hitpoint: hit point if intersected
         */
        bool intersect(const Ray& ray, Hitpoint* hitpoint) const override;

        /** Area of the Trimesh.
         */
        double area() const override;

        /** Triangulate the mesh.
         */
        std::vector<Triangle> triangulate() const override;

        /** Build accelerator structure.
         */
        void buildAccel(AccelType accelType);

        /** Load mesh file
         * @param[in] filename: Mesh file of .PLY or .OBJ format.
         */
        void load(const std::string& filename);

        /** Resize vertex and face arrays.
         */
        void resize(int numVerts, int numFaces);

        /** Translate the mesh.
         * @param[in] move: Transtion delta.
         */
        void translate(const Vector3D& move);

        /** Scale the mesh.
         *  @param[in] scaleX: scaling along X-axis.
         *  @param[in] scaleY: scaling along Y-axis.
         *  @param[in] scaleZ: scaling along Z-axis.
         */
        void scale(const double scaleX, const double scaleY, const double scaleZ);

        /** Scale the mesh.
         *  @param[in] scaleAll: scaling all the axes.
         */
        void scale(const double scaleAll);

        /** Rotate the mesh along specified axis.
         *  @param theta: Rotation angle by radian.
         *  @param axis: Rotation axis.
         *  @param origin: Origin of the axis.
         */
        void rotate(double theta, const Vector3D& axis,
                    const Vector3D& origin = Vector3D(0.0, 0.0, 0.0));

        /** Put the mesh on the specified plane.
         *  @param[in] plane: a plane on which the mesh is put.
         */
        void putOnPlane(const Plane& plane);

        /** Scale and translate the mesh to fit the bounding box.
         *  @param[in] bbox: target bounding box.
         */
        void fitToBBox(const BBox& bbox);

        /** Get a triangle with specified ID.
         *  @param[in] faceID: ID of the triangle to get.
         */
        Triangle getTriangle(int faceID) const;

        /** Get vertex indices.
         */
        const std::vector<Triplet>& getIndices() const;

        /** Get the vertex with specified ID.
         *  @param[in] vertexID: ID of the vertex.
         */
        const Vector3D& getVertex(int vertexID) const;

        /** Assign a vertex position to the specified ID.
         */
        void setVertex(int vertexID, const Vector3D& v);

        /** Get the vertex with specified ID.
         */
        const VertexData& getVertexData(int vertexID) const;

        /** Assign a face indices to the specified ID.
         */
        void setFace(int faceID, const Triplet& face);

        /** Get the texture coordinates with specified ID.
         */
        const Vector2D& getTexcoord(int vertexID) const;

        /** Assign a texture coordinates to the specified ID.
         */
        void setTexcoord(int vertexID, const Vector2D& texcoord);

        /** Set color to the specified vertex.
         *  @param[in] vertexID: ID of the vertex.
         */
        void setColor(int vertexID, const Color& color);

        /** Get the color of the vertex with specified ID.
         *  @param[in] vertexID: ID of the vertex.
         */
        Color getColor(int vertexID) const;

        /** Get the normal of a face with specified ID.
         *  @param[in] faceID: ID of the triangle to get a normal.
         */
        Vector3D getNormal(int faceID) const;

        /** Get the number of vertices
         */
        inline decltype(auto) numVerts() const { return _vertices.size(); }

        /** Get the number of faces (triangles).
         */
        inline decltype(auto) numFaces() const { return _faces.size(); }

        /** Set texture image.
         */
        void setTexture(const Image& image);

        /** Trimesh has texture or not.
         */
        inline bool isTextured() const { return static_cast<bool>(_texture); } 

    private:
        /** Compute vertex normals.
         */
        void calcVertexNormals();

        friend class Scene;
    };

}  // namespace spica

#endif  // _SPICA_TRIMESH_H_
