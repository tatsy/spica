#define SPICA_SCENE_EXPORT
#include "scene.h"

#include "../renderer/brdf.h"
#include "../camera/camera.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _triangles()

        , _bsdfIds()
        , _lightIds()

        , _bsdfs()
        , _accel()
        , _accelType(AccelType::qbvhAccel) {
    }

    Scene::~Scene() {
    }

    Scene::Scene(const Scene& scene)
        : _triangles()

        , _bsdfIds()
        , _lightIds()

        , _bsdfs()
        , _accel()
        , _accelType(AccelType::qbvhAccel) {
        this->operator=(scene);
    }


    Scene& Scene::operator=(const Scene& scene) {
        this->_triangles = scene._triangles;

        this->_bsdfIds   = scene._bsdfIds;
        this->_lightIds  = scene._lightIds;
        
        this->_bsdfs = scene._bsdfs;
        this->_accel = scene._accel;
        this->_accelType = scene._accelType;
                
        return *this;
    }

    Sphere Scene::boundingSphere(const Camera& camera) const {
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
        return Sphere(radius, center);
    }

    void Scene::setEnvmap(const std::string& filename, const Camera& camera) {
        const Image image = Image::fromFile(filename);
        setEnvmap(image, camera);
    }

    void Scene::setEnvmap(const Image& image, const Camera& camera) {
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

    const Triangle& Scene::getTriangle(int id) const {
        Assertion(id >= 0 && id < _triangles.size(),
                  "Object index out of bounds");

        const Vector3D& v0 = _vertices[_triangles[id][0]].pos();
        const Vector3D& v1 = _vertices[_triangles[id][1]].pos();
        const Vector3D& v2 = _vertices[_triangles[id][2]].pos();
        return Triangle(v0, v1, v2);
    }

    const BSDF& Scene::getBsdf(int id) const {
        Assertion(id >= 0 && id < _bsdfIds.size(),
                  "Object index out of boudns");
        return _bsdfs[_bsdfIds[id]];
    }

    Color Scene::directLight(const Vector3D& dir) const {
        return _lighting.directLight(dir);
    }

    LightSample Scene::sampleLight(Stack<double>& rstack) const {
        return _lighting.sample(rstack);
    }

    double Scene::lightArea() const {
        return _lighting.area();
    }

    void Scene::clear() {
        _triangles.clear();
        _triangles.shrink_to_fit();
        _bsdfIds.clear();
        _bsdfIds.shrink_to_fit();
        _bsdfs.clear();
        _bsdfs.shrink_to_fit();
    }

    void Scene::setAccelType(AccelType type) {
        this->_accelType = type;
    }

    bool Scene::isLightCheck(int id) const {
        auto it = std::lower_bound(_lightIds.begin(), _lightIds.end(), id);
        return it != _lightIds.end() && (*it) == id;
    }

    void Scene::computeAccelerator() {
        switch (this->_accelType) {
        case AccelType::bbvhAccel:
            _accel = std::shared_ptr<AccelBase>(new BBVHAccel());
            break;

        case AccelType::qbvhAccel:
            _accel = std::shared_ptr<AccelBase>(new QBVHAccel());
            break;
        
        case AccelType::kdtreeAccel:
            _accel = std::shared_ptr<AccelBase>(new KdTreeAccel());
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

    void Scene::finalize() {
        this->computeAccelerator();
    }

    bool Scene::intersect(const Ray& ray, Intersection& isect) const {
        Assertion(_accel, "Accelerator is not prepared !!");

        Hitpoint hitpoint;
        const int triID = _accel->intersect(ray, &hitpoint);

        if (_triangles[triID].isTextured()) {
            double u = hitpoint.texcoord().x();
            double v = hitpoint.texcoord().y();
            const Vector2D t0 = _vertices[_triangles[triID][0]].texcoord();
            const Vector2D t1 = _vertices[_triangles[triID][1]].texcoord();
            const Vector2D t2 = _vertices[_triangles[triID][2]].texcoord();
            hitpoint.setTexcoord(t0 + u * (t1 - t0) + v * (t2 - t0));
        }

        isect.setObjectId(triID);
        isect.setHitpoint(hitpoint);

        return triID != -1;
    }

    void Scene::addTriangles(const std::vector<Triangle>& tris) {
        const int newTris = static_cast<int>(tris.size());
        for (int i = 0; i < newTris; i++) {
            const int vid = static_cast<int>(_vertices.size());
            for (int k = 0; k < 3; k++) {
                _vertices.emplace_back(tris[i][k]);
            }
            _triangles.emplace_back(vid, vid + 1, vid + 2);
        }    
    }

    void Scene::addBsdf(const BSDF& bsdf, int numTris) {
        const int newBsdfId = static_cast<int>(_bsdfs.size());
        const int curTris = static_cast<int>(_bsdfIds.size());
        _bsdfIds.resize(_bsdfIds.size() + numTris);
        std::fill(_bsdfIds.begin() + curTris, _bsdfIds.end(), newBsdfId);
        _bsdfs.push_back(bsdf);
    }

}  // namespace spica
