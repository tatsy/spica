#define SPICA_SCENE_EXPORT
#include "scene.h"

#include "brdf.h"
#include "../camera/camera.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _triangles()
        , _emittance()

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
        , _emittance()

        , _bsdfIds()
        , _lightIds()

        , _bsdfs()
        , _accel()
        , _accelType(AccelType::qbvhAccel) {
        this->operator=(scene);
    }


    Scene& Scene::operator=(const Scene& scene) {
        this->_triangles = scene._triangles;
        this->_emittance = scene._emittance;

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
            center += _triangles[i].gravity();
        }
        center /= _triangles.size();

        double radius = (center - camera.center()).norm();
        for (int i = 0; i < _triangles.size(); i++) {
            for (int k = 0; k < 3; k++) {
                double d = (center - _triangles[i][k]).norm();
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

        // Both normal geoms and light geoms are set to triangles
        _triangles.insert(_triangles.end(), tris.begin(), tris.end());

        // Add empty BSDF
        const int bsdfID = static_cast<int>(_bsdfs.size());
        _bsdfIds.resize(_bsdfIds.size() + tris.size());
        std::fill(_bsdfIds.begin() + nowTris, _bsdfIds.end(), bsdfID);
        _bsdfs.push_back(LambertianBRDF::factory(Color(0.0, 0.0, 0.0)));        
    }

    const Triangle& Scene::getTriangle(int id) const {
        Assertion(id >= 0 && id < _triangles.size(),
                  "Object index out of bounds");
        return _triangles[id];
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
        _accel->construct(_triangles);    
    }

    void Scene::finalize() {
        this->computeAccelerator();
    }

    bool Scene::intersect(const Ray& ray, Intersection& isect) const {
        Assertion(_accel, "Accelerator is not prepared !!");

        Hitpoint hitpoint;
        const int triID = _accel->intersect(ray, &hitpoint);

        isect.setObjectId(triID);
        isect.setHitpoint(hitpoint);
        return triID != -1;
    }

}  // namespace spica
