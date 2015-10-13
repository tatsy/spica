#define SPICA_SCENE_EXPORT
#include "scene.h"

#include <cstring>

#include "../core/color.h"
#include "../math/vector3d.h"
#include "../accel/accel.h"
#include "../camera/camera.h"
#include "../light/lighting.h"
#include "../bsdf/bsdf.h"
#include "../bsdf/brdf.h"
#include "../shape/shape.h"

#include "../accel/bbvh_accel.h"
#include "../accel/qbvh_accel.h"
#include "../accel/kd_tree_accel.h"

#include "vertex_data.h"
#include "triangle_data.h"

namespace spica {

    class Scene::SceneImpl : private Uncopyable {
    private:
        std::vector<VertexData>   _vertices;
        std::vector<TriangleData> _triangles;

        std::vector<unsigned int> _bsdfIds;
        std::vector<unsigned int> _lightIds;

        std::vector<BSDF>          _bsdfs;
        std::shared_ptr<IAccel> _accel;

        Lighting  _lighting;
        AccelType _accelType;

        std::shared_ptr<Image> _texture;

    public:
        SceneImpl()
            : _vertices{}
            , _triangles{}
            , _bsdfIds{}
            , _lightIds{}
            , _bsdfs{}
            , _accel{}
            , _lighting{}
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
            this->_lightIds  = std::move(impl._lightIds);
            this->_bsdfs     = std::move(impl._bsdfs);
            this->_accel     = std::move(impl._accel);
            this->_lighting  = std::move(impl._lighting);
            this->_accelType = impl._accelType;
            this->_texture   = impl._texture;
            return *this;
        }

        void clear() {
            _vertices.clear();
            _triangles.clear();
            _bsdfIds.clear();
            _lightIds.clear();
            _bsdfs.clear();
        }

        Sphere boundingSphere(const Camera& camera) {
            Vector3D center;
            for (int i = 0; i < _triangles.size(); i++) {
                const Vector3D& v0 = _vertices[_triangles[i][0]].pos();
                const Vector3D& v1 = _vertices[_triangles[i][1]].pos();
                const Vector3D& v2 = _vertices[_triangles[i][2]].pos();
                center += (v0 + v1 + v2) / 3.0;
            }
            center /= _triangles.size();

            double radius = (center - camera.center()).norm();
            for (int i = 0; i < _triangles.size(); i++) {
                for (int k = 0; k < 3; k++) {
                    const Vector3D& v = _vertices[_triangles[i][k]].pos();
                    double d = (center - v).norm();
                    radius = std::max(radius, d);
                }
            }
            return Sphere(center, radius);
        }

        void setEnvmap(const Image& image, const Camera& camera) {
            const Sphere& shape = this->boundingSphere(camera);
            std::vector<Triangle> tris = shape.triangulate();
            const int newTris = static_cast<int>(tris.size());
            const int nowTris = static_cast<int>(_triangles.size());
            _lighting = Lighting::asEnvmap(shape, image);

            // If new object is a light, store triangle indices
            for (int i = 0; i < newTris; i++) {
                _lightIds.push_back(nowTris + i);
            }

            // Add triangles to the scene
            addTriangles(tris);

            // Add empty BSDF
            addBsdf(LambertianBRDF::factory(Color(0.0, 0.0, 0.0)), newTris);
        }

        Triangle getTriangle(int id) const {
            Assertion(id >= 0 && id < _triangles.size(),
                      "Object index out of bounds");

            const Vector3D& v0 = _vertices[_triangles[id][0]].pos();
            const Vector3D& v1 = _vertices[_triangles[id][1]].pos();
            const Vector3D& v2 = _vertices[_triangles[id][2]].pos();
            return Triangle(v0, v1, v2);
        }

        const BSDF& getBsdf(int id) const {
            Assertion(id >= 0 && id < _bsdfIds.size(),
                      "Object index out of boudns");
            return _bsdfs[_bsdfIds[id]];
        }

        Color directLight(const Vector3D& dir) const {
            return _lighting.directLight(dir);
        }

        LightSample sampleLight(Stack<double>& rstack) const {
            return _lighting.sample(rstack);
        }

        double lightArea() const {
            return _lighting.area();
        }

        void setAccelType(AccelType type) {
            this->_accelType = type;
        }

        bool isLightCheck(int id) const {
            auto it = std::lower_bound(_lightIds.cbegin(), _lightIds.cend(), id);
            return it != _lightIds.cend() && (*it) == id;
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
                const Vector3D& v0 = _vertices[_triangles[i][0]].pos();
                const Vector3D& v1 = _vertices[_triangles[i][1]].pos();
                const Vector3D& v2 = _vertices[_triangles[i][2]].pos();
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
                Color color;
                if (_triangles[triID].isTextured()) {
                    double u = hitpoint.texcoord().x();
                    double v = hitpoint.texcoord().y();
                    const Vector2D t0 = _vertices[_triangles[triID][0]].texcoord();
                    const Vector2D t1 = _vertices[_triangles[triID][1]].texcoord();
                    const Vector2D t2 = _vertices[_triangles[triID][2]].texcoord();
                    const Vector2D uv = t0 + u * (t1 - t0) + v * (t2 - t0);
                    const int tx = uv.x() * _texture->width();
                    const int ty = uv.y() * _texture->height();
                    color = _texture->pixel(tx, ty);
                } else {
                    color = getBsdf(triID).reflectance();
                }
                (*isect) = Intersection(triID, hitpoint, color);
            } else {
                (*isect) = Intersection();
            }
            return triID != -1;
        }

        bool isTextured(int triID) const {
            Assertion(triID >= 0 && triID < _triangles.size(), "Triangle index out of bounds!!");
            return _triangles[triID].isTextured();
        }

        int numTriangles() const {
            return static_cast<int>(_triangles.size());
        }

        void addShape(const std::vector<Triangle>& tris, const BSDF& bsdf) {
            addTriangles(tris);
            addBsdf(bsdf, tris.size());
        }

        void addShape(const Trimesh& shape, const BSDF& bsdf) {
            // Copy triangles
            std::vector<Triplet> trip = shape.getIndices();
            for (int i = 0; i < trip.size(); i++) {
                const int vid = _vertices.size();
                for (int k = 0; k < 3; k++) {
                    _vertices.push_back(shape.getVertexData(trip[i][k]));
                }
                _triangles.emplace_back(vid, vid + 1, vid + 2, shape.isTextured());
            }

            _texture = shape._texture;

            // Update BSDF ids
            addBsdf(bsdf, trip.size());
        }

        void setAreaLight(const std::vector<Triangle>& tris, const Color& emission) {
            addTriangles(tris);

            // Set as area light
            _lighting = Lighting::asAreaLight(tris, emission);

            // If new object is a light, store triangle indices
            const int newTris = static_cast<int>(tris.size());
            const int nowTris = static_cast<int>(_triangles.size()) - newTris;
            for (int i = 0; i < newTris; i++) {
                _lightIds.push_back(nowTris + i);
            }

            // Add empty BSDF
            addBsdf(LambertianBRDF::factory(Color(0.0, 0.0, 0.0)), newTris);
        }

    private:
        void addTriangles(const std::vector<Triangle>& tris) {
            const int newTris = static_cast<int>(tris.size());
            for (int i = 0; i < newTris; i++) {
                const int vid = static_cast<int>(_vertices.size());
                for (int k = 0; k < 3; k++) {
                    _vertices.emplace_back(tris[i][k]);
                }
                _triangles.emplace_back(vid, vid + 1, vid + 2);
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

    Sphere Scene::boundingSphere(const Camera& camera) const {
        return _impl->boundingSphere(camera);
    }

    void Scene::setEnvmap(const Image& image, const Camera& camera) {
        _impl->setEnvmap(image, camera);
    }

    Triangle Scene::getTriangle(int id) const {
        return _impl->getTriangle(id);
    }

    const BSDF& Scene::getBsdf(int id) const {
        return _impl->getBsdf(id);
    }

    Color Scene::directLight(const Vector3D& dir) const {
        return _impl->directLight(dir);
    }

    LightSample Scene::sampleLight(Stack<double>& rstack) const {
        return _impl->sampleLight(rstack);
    }

    double Scene::lightArea() const {
        return _impl->lightArea();
    }

    void Scene::setAccelType(AccelType type) {
        _impl->setAccelType(type);
    }

    bool Scene::isLightCheck(int id) const {
        return _impl->isLightCheck(id);
    }

    void Scene::computeAccelerator() {
        _impl->computeAccelerator();
    }

    void Scene::finalize() {
        _impl->finalize();
    }

    bool Scene::intersect(const Ray& ray, Intersection* isect) const {
        return _impl->intersect(ray, isect);
    }

    bool Scene::isTextured(int triID) const {
        return _impl->isTextured(triID);
    }

    int Scene::numTriangles() const {
        return _impl->numTriangles();
    }

    template <>
    SPICA_SCENE_DLL
    void Scene::addShape(const BBox& shape, const BSDF& bsdf) {
        _impl->addShape(shape.triangulate(), bsdf);
    }

    template <>
    SPICA_SCENE_DLL
        void Scene::addShape(const Disk& shape, const BSDF& bsdf) {
        _impl->addShape(shape.triangulate(), bsdf);
    }

    template <>
    SPICA_SCENE_DLL
        void Scene::addShape(const Quad& shape, const BSDF& bsdf) {
        _impl->addShape(shape.triangulate(), bsdf);
    }

    template <>
    SPICA_SCENE_DLL
    void Scene::addShape(const Sphere& shape, const BSDF& bsdf) {
        _impl->addShape(shape.triangulate(), bsdf);
    }

    template <>
    SPICA_SCENE_DLL
        void Scene::addShape(const Triangle& shape, const BSDF& bsdf) {
        _impl->addShape(shape.triangulate(), bsdf);
    }

    template <>
    SPICA_SCENE_DLL
        void Scene::addShape(const Trimesh& shape, const BSDF& bsdf) {
        _impl->addShape(shape, bsdf);
    }

    template <>
    SPICA_SCENE_DLL
        void Scene::setAreaLight(const BBox& shape, const Color& emission) {
        _impl->setAreaLight(shape.triangulate(), emission);    
    }

    template <>
    SPICA_SCENE_DLL
        void Scene::setAreaLight(const Disk& shape, const Color& emission) {
        _impl->setAreaLight(shape.triangulate(), emission);
    }

    template <>
    SPICA_SCENE_DLL
    void Scene::setAreaLight(const Quad& shape, const Color& emission) {
        _impl->setAreaLight(shape.triangulate(), emission);
    }

    template <>
    SPICA_SCENE_DLL
    void Scene::setAreaLight(const Sphere& shape, const Color& emission) {
        _impl->setAreaLight(shape.triangulate(), emission);
    }

    template <>
    SPICA_SCENE_DLL
    void Scene::setAreaLight(const Triangle& shape, const Color& emission) {
        _impl->setAreaLight(shape.triangulate(), emission);
    }

    template <>
    SPICA_SCENE_DLL
    void Scene::setAreaLight(const Trimesh& shape, const Color& emission) {
        _impl->setAreaLight(shape.triangulate(), emission);
    }

}  // namespace spica
