#define SPICA_SCENE_EXPORT
#include "scene.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _nIGeometrys(0)
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
        if (_nIGeometrys == _arraySize) {
            _arraySize *= 2;
            const IGeometry** primPtr = new const IGeometry*[_arraySize];
            Material* matPtr = new Material[_arraySize];
            memcpy((void*)primPtr, (void*)_primitives, sizeof(IGeometry*) * _nIGeometrys);
            memcpy((void*)matPtr, (void*)_materials, sizeof(Material) * _nIGeometrys);
            release();
            _primitives = primPtr;
            _materials = matPtr;
        }
    }

    const IGeometry* Scene::get(int id) const {
        Assertion(id >= 0 && id < _nIGeometrys, "Object index out of bounds");
        return _primitives[id];
    }

    const Material& Scene::getMaterial(int id) const {
        Assertion(id >= 0 && id < _nIGeometrys, "Object index out of boudns");
        return _materials[id];
    }

    void Scene::init() {
        this->_nIGeometrys = 0;
        this->_arraySize = 1024;
        _primitives = new const IGeometry*[_arraySize];
        _materials = new Material[_arraySize];
        _envmap.resize(512, 512);
        _envmap.clearColor(Color(0.0, 0.0, 0.0));
    }

    void Scene::clear() {
        release();
        init();
    }

    void Scene::release() {
        for (int i = 0; i < _nIGeometrys; i++) {
            delete _primitives[i];
        }
        delete[] _primitives;    
        delete[] _materials;
    }

    bool Scene::intersect(const Ray& ray, Intersection& isect) const {
        // Linear search
        int objID = -1;
        Hitpoint hitpoint;
        for (int i = 0; i < _nIGeometrys; i++) {
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
