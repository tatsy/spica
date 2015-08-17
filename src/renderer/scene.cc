#define SPICA_SCENE_EXPORT
#include "scene.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _triangles()
        , _lights()
        , _bsdfs()
        , _bsdfIds()
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

    bool Scene::intersect(const Ray& ray, Intersection& isect) const {
        Assertion(_accel, "Accelerator is not prepared !!");

        Hitpoint hitpoint;
        const int triID = _accel->intersect(ray, &hitpoint);

        isect.setObjectId(triID);
        isect.setHitpoint(hitpoint);
        return triID != -1;
    }

}  // namespace spica
