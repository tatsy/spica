#define SPICA_SCENE_EXPORT
#include "scene.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _triangles()
        , _emittance()

        , _bsdfIds()
        , _lightIds()
        , _lightPdfs()
        , _totalLightArea(0.0)

        , _bsdfs()
        , _accel()
        , _accelType(AccelType::qbvhAccel)
    {
    }

    Scene::~Scene()
    {
    }

    Scene::Scene(const Scene& scene)
        : _triangles()
        , _emittance()

        , _bsdfIds()
        , _lightIds()
        , _lightPdfs()
        , _totalLightArea(0.0)

        , _bsdfs()
        , _accel()
        , _accelType(AccelType::qbvhAccel)
    {
        this->operator=(scene);
    }


    Scene& Scene::operator=(const Scene& scene) {
        this->_triangles = scene._triangles;
        this->_emittance = scene._emittance;

        this->_bsdfIds   = scene._bsdfIds;
        this->_lightIds  = scene._lightIds;
        
        this->_totalLightArea = scene._totalLightArea;
        this->_bsdfs = scene._bsdfs;
        this->_accel = scene._accel;
        this->_accelType = scene._accelType;
                
        return *this;
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

    const Color& Scene::getEmittance(int id) const {
        Assertion(id >= 0 && id < _emittance.size(),
                  "Object index out of boudns");
        return _emittance[id];    
    }

    int Scene::sampleLight(double rand1) const {
        Assertion(!_lightPdfs.empty(), 
                  "Light PDFs are not computed yet, "
                  "Scene::computeLightPdfs first!!");
        
        const int id = std::lower_bound(_lightPdfs.begin(), _lightPdfs.end(), rand1) - _lightPdfs.begin();
        return _lightIds[id];
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

    void Scene::computeLightPdfs() {
        if (_lightIds.empty()) return; 

        _lightPdfs.resize(_lightIds.size());

        _totalLightArea = 0.0;
        for (int i = 0; i < _lightIds.size(); i++) {
            const double A = _triangles[_lightIds[i]].area();
            _lightPdfs[i] = A;
            _totalLightArea += A;
        }

        _lightPdfs[0] /= _totalLightArea;
        for (int i = 1; i < _lightIds.size(); i++) {
            _lightPdfs[i] = _lightPdfs[i - 1] + 
                            _lightPdfs[i] / _totalLightArea;
        }
    }

    void Scene::finalize() {
        this->computeAccelerator();
        this->computeLightPdfs();
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
