#define SPICA_SCENE_EXPORT
#include "scene.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _nPrimitives(0)
        , _arraySize(0)
        , _lightID(-1)
        , _primitives(0)
        , _materials(0)
        , _envmap()
    {
        init();
    }

    Scene::~Scene()
    {
        release();
    }

    void Scene::checkArraySize() {
        if (_nPrimitives == _arraySize) {
            _arraySize *= 2;
            const Primitive** primPtr = new const Primitive*[_arraySize];
            Material* matPtr = new Material[_arraySize];
            memcpy((void*)primPtr, (void*)_primitives, sizeof(Primitive*) * _nPrimitives);
            memcpy((void*)matPtr, (void*)_materials, sizeof(Material) * _nPrimitives);
            release();
            _primitives = primPtr;
            _materials = matPtr;
        }
    }

    const Primitive* Scene::get(int id) const {
        msg_assert(id >= 0 && id < _nPrimitives, "Object index out of bounds");
        return _primitives[id];
    }

    const Material& Scene::getMaterial(int id) const {
        msg_assert(id >= 0 && id < _nPrimitives, "Object index out of boudns");
        return _materials[id];
    }

    void Scene::init() {
        this->_nPrimitives = 0;
        this->_arraySize = 1024;
        _primitives = new const Primitive*[_arraySize];
        _materials = new Material[_arraySize];
        _envmap.resize(512, 512);
        _envmap.clearColor(Color(0.0, 0.0, 0.0));
    }

    void Scene::clear() {
        release();
        init();
    }

    void Scene::release() {
        for (int i = 0; i < _nPrimitives; i++) {
            delete _primitives[i];
        }
        delete[] _primitives;    
        delete[] _materials;
    }

    bool Scene::intersect(const Ray& ray, Intersection& isect) const {
        // Linear search
        int objID = -1;
        Hitpoint hitpoint;
        for (int i = 0; i < _nPrimitives; i++) {
            Hitpoint hpTemp;
            if (_primitives[i]->intersect(ray, &hpTemp)) {
                if (hitpoint.distance() > hpTemp.distance()) {
                    objID = i;
                    hitpoint = hpTemp;
                }
            }
        }

        isect.setObjectId(objID);
        isect.setHitpoint(hitpoint);
        return objID != -1;
    }

}  // namespace spica
