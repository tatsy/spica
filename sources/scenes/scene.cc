#define SPICA_API_EXPORT
#include "scene.h"

#include <cstring>

#include "../core/spectrum.h"
#include "../math/vector3d.h"
#include "../accel/accel.h"
#include "../camera/camera.h"

#include "../light/envmap.h"

#include "../bsdf/bsdf.h"
#include "../bsdf/brdf.h"
#include "../shape/shape.h"

#include "../accel/bbvh_accel.h"
#include "../accel/qbvh_accel.h"
#include "../accel/kd_tree_accel.h"

#include "../renderer/photon_map.h"

#include "vertex_data.h"
#include "triangle_data.h"

namespace spica {

    class Scene::SceneImpl : private Uncopyable {
    private:
        std::vector<VertexData>   _vertices;
        std::vector<TriangleData> _triangles;

        std::vector<unsigned int> _bsdfIds;
        std::vector<std::shared_ptr<Light> > lights;

        std::vector<BSDF>       _bsdfs;
        std::shared_ptr<IAccel> _accel;

        AccelType _accelType;

        std::shared_ptr<Image> _texture;

    public:
        SceneImpl()
            : _vertices{}
            , _triangles{}
            , _bsdfIds{}
            , lights{}
            , _bsdfs{}
            , _accel{}
            , _accelType{AccelType::QBVH}
            , _texture{} {
        }

        ~SceneImpl() {
        }

        SceneImpl(SceneImpl&& impl) 
            : SceneImpl{} {
            this->operator=(std::move(impl));
        }

        SceneImpl& operator=(SceneImpl&& impl) {
            this->_vertices  = std::move(impl._vertices);
            this->_triangles = std::move(impl._triangles);
            this->_bsdfIds   = std::move(impl._bsdfIds);
            this->_bsdfs     = std::move(impl._bsdfs);
            this->_accel     = std::move(impl._accel);
            this->_accelType = impl._accelType;
            this->_texture   = impl._texture;
            return *this;
        }

        void clear() {
            _vertices.clear();
            _triangles.clear();
            _bsdfIds.clear();
            lights.clear();
            _bsdfs.clear();
        }

        Sphere boundingSphere() {
            Point center(0.0, 0.0, 0.0);
            for (int i = 0; i < _triangles.size(); i++) {
                const Point& v0 = _vertices[_triangles[i][0]].pos();
                const Point& v1 = _vertices[_triangles[i][1]].pos();
                const Point& v2 = _vertices[_triangles[i][2]].pos();
                center += (v0 + v1 + v2) / 3.0;
            }
            center /= _triangles.size();

            double radius = 0.0;
            for (int i = 0; i < _triangles.size(); i++) {
                for (int k = 0; k < 3; k++) {
                    const Point& v = _vertices[_triangles[i][k]].pos();
                    double d = (center - v).norm();
                    radius = std::max(radius, d);
                }
            }
            return Sphere(center, radius);
        }

        void computeAccelerator() {
            switch (this->_accelType) {
            case AccelType::BBVH:
                _accel = std::make_shared<BBVHAccel>();
                break;

            case AccelType::QBVH:
                _accel = std::make_shared<QBVHAccel>();
                break;

            case AccelType::KdTree:
                _accel = std::make_shared<KdTreeAccel>();
                break;

            default:
                std::cerr << "[ERROR] unknown accelerator type !!" << std::endl;
                std::abort();
            }

            std::vector<Triangle> tris(_triangles.size());
            for (int i = 0; i < _triangles.size(); i++) {
                const Point& v0 = _vertices[_triangles[i][0]].pos();
                const Point& v1 = _vertices[_triangles[i][1]].pos();
                const Point& v2 = _vertices[_triangles[i][2]].pos();
                tris[i] = Triangle(v0, v1, v2);
            }
            _accel->construct(tris);
        }

        void finalize() {
            this->computeAccelerator();
        }

        bool intersect(const Ray& ray, Intersection* isect) {
            Assertion(_accel, "Accelerator is not prepared !!");

            Hitpoint hitpoint;
            const int triID = _accel->intersect(ray, &hitpoint);

            if (triID != -1) {
                Spectrum color;
                double u = hitpoint.texcoord().x();
                double v = hitpoint.texcoord().y();
                if (_triangles[triID].isTextured()) {
                    // Compute texture coordinates
                    const Vector2D t0 = _vertices[_triangles[triID][0]].texcoord();
                    const Vector2D t1 = _vertices[_triangles[triID][1]].texcoord();
                    const Vector2D t2 = _vertices[_triangles[triID][2]].texcoord();
                    const Vector2D uv = t0 + u * (t1 - t0) + v * (t2 - t0);
                    const int tx = uv.x() * (_texture->width() - 1);
                    const int ty = uv.y() * (_texture->height() - 1);
                    color = _texture->pixel(tx, ty);
                } else {
                    //color = getBsdf(triID).reflectance();
                }

                const Normal n0 = _vertices[_triangles[triID][0]].normal();
                const Normal n1 = _vertices[_triangles[triID][1]].normal();
                const Normal n2 = _vertices[_triangles[triID][2]].normal();
                const Normal n  = (n0 + u * (n1 - n0) + v * (n2 - n0)).normalized();

                hitpoint.setNormal(n);
                (*isect) = Intersection(triID, hitpoint, color);
            } else {
                (*isect) = Intersection();
            }
            return triID != -1;
        }

        void addShape(const std::vector<Triangle>& tris, const BSDF& bsdf) {
            addTriangles(tris);
            addBsdf(bsdf, tris.size());
        }

        void addLight(const std::shared_ptr<Light>& light) {
            lights.push_back(light);
        }

    private:
        void addTriangles(const Trimesh& tris) {
            const int vid = static_cast<int>(_vertices.size());
            for (int i = 0; i < tris.numVerts(); i++) {
                _vertices.push_back(tris.getVertexData(i));
            }

            const std::vector<Triplet> faces = tris.getIndices();
            for (int i = 0; i < faces.size(); i++) {
                _triangles.emplace_back(vid + faces[i][0], vid + faces[i][1], vid + faces[i][2]);
            }
        }

        void addBsdf(const BSDF& bsdf, int numTris) {
            const int newBsdfId = static_cast<int>(_bsdfs.size());
            const int curTris   = static_cast<int>(_bsdfIds.size());
            _bsdfIds.resize(_bsdfIds.size() + numTris);
            std::fill(_bsdfIds.begin() + curTris, _bsdfIds.end(), newBsdfId);
            _bsdfs.push_back(bsdf);
        }

    };  // class SceneImpl
    
    Scene::Scene()
        :_impl{std::make_unique<SceneImpl>()} {
    }

    Scene::Scene(Scene&& scene)
        : _impl{std::move(scene._impl)} {
    }

    Scene::~Scene() {
    }

    Scene& Scene::operator=(Scene&& scene) {
        this->_impl = std::move(scene._impl);
        return *this;
    }

    void Scene::clear() {
        _impl->clear();
    }

    Sphere Scene::boundingSphere() const {
        return _impl->boundingSphere();
    }

    void Scene::addLight(const std::shared_ptr<Light>& light) {
        _impl->addLight(light);
    }

}  // namespace spica
