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
        , _envmap()
    {
        _envmap.resize(512, 512);
        _envmap.clearColor(Color::BLACK);
    }

    Scene::~Scene()
    {
    }

    const Triangle& Scene::getTriangle(int id) const {
        Assertion(id >= 0 && id < _triangles.size(), "Object index out of bounds");
        return _triangles[id];
    }

    const BSDF& Scene::getBsdf(int id) const {
        Assertion(id >= 0 && id < _bsdfIds.size(), "Object index out of boudns");
        return _bsdfs[_bsdfIds[id]];
    }

    const Color& Scene::getEmittance(int id) const {
        Assertion(id >= 0 && id < _bsdfIds.size(), "Object index out of boudns");
        return _emittance[id];    
    }

    int Scene::sampleLight(double rand1) const {
        Assertion(!_lightPdfs.empty(), "Light PDFs are not computed yet, call Scene::computeLightPdfs first!!");
        
        int lo = 0;
        int hi = _lightIds.size() - 1;
        while (lo < hi) {
            const int mid = (lo + hi) / 2;
            if (_lightPdfs[mid] < rand1) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    void Scene::clear() {
        _triangles.clear();
        _triangles.shrink_to_fit();
        _bsdfIds.clear();
        _bsdfIds.shrink_to_fit();
        _bsdfs.clear();
        _bsdfs.shrink_to_fit();
    }

    void Scene::setAccelerator(AccelType type) {
        switch (type) {
        case QBVH_ACCEL:
            _accel = std::shared_ptr<AccelBase>(new QBVHAccel());
            break;
        
        case KD_TREE_ACCEL:
            _accel = std::shared_ptr<AccelBase>(new KdTreeAccel());
            break;
        
        default:
            std::cerr << "[ERROR] unknown accelerator type !!" << std::endl;
            std::abort();
        }
        _accel->construct(_triangles);
    }

    void Scene::computeLightPdfs() {        
        _lightPdfs.resize(_lightIds.size());

        _totalLightArea = 0.0;
        for (int i = 0; i < _lightIds.size(); i++) {
            const double A = _triangles[_lightIds[i]].area();
            _lightPdfs[i] = A;
            _totalLightArea += A;
        }

        _lightPdfs[0] /= _totalLightArea;
        for (int i = 1; i < _lightIds.size(); i++) {
            _lightPdfs[i] += _lightPdfs[i - 1] + _lightPdfs[i] / _totalLightArea;
        }
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
