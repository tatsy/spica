#define SPICA_SCENE_EXPORT
#include "scene.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _nPrimitives(0)
        , _arraySize(0)
        , _primitives(0)
        , _bgColor(0.0, 0.0, 0.0)
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
            Primitive** ptr = new Primitive*[_arraySize];
            memcpy(ptr, _primitives, sizeof(Primitive*) * _nPrimitives);
            release();
            _primitives = ptr;
        }
    }

    void Scene::addPlane(const Plane& plane, bool isLight) {
        if (isLight) _lightID = _nPrimitives;
        _primitives[_nPrimitives++] = new Plane(plane);
        checkArraySize();
    }

    void Scene::addSphere(const Sphere& sphere, bool isLight) {
        if (isLight) _lightID = _nPrimitives;
        _primitives[_nPrimitives++] = new Sphere(sphere);
        checkArraySize();
    }

    void Scene::addTrimesh(const Trimesh& trimesh, bool isLight) {
        if (isLight) _lightID = _nPrimitives;
        Trimesh* ptr = new Trimesh(trimesh);
        ptr->buildKdTreeAccel();
        _primitives[_nPrimitives++] = ptr;
        checkArraySize();
    }

    const Primitive* Scene::getObjectPtr(int id) const {
        msg_assert(id >= 0 && id < _nPrimitives, "Object index out of bounds");
        return _primitives[id];
    }

    void Scene::init() {
        this->_nPrimitives = 0;
        this->_arraySize = 1024;
        _primitives = new Primitive*[_arraySize];
        _bgColor = Color(0.0, 0.0, 0.0);
    }

    void Scene::clear() {
        release();
        init();
    }

    void Scene::release() {
        for (int i = 0; i < _nPrimitives; i++) {
            delete _primitives[i];
        }
        delete _primitives;    
    }

    bool Scene::intersect(const Ray& ray, Intersection& intersection) const {
        // Linear search
        for (int i = 0; i < _nPrimitives; i++) {
            HitPoint hitpoint;
            if (_primitives[i]->intersect(ray, hitpoint)) {
                if (hitpoint.distance() < intersection.hittingDistance()) {
                    intersection.setHitPoint(hitpoint);
                    intersection.setObjectId(i);
                }
            }
        }
        return (intersection.objectId() != -1);
    }
}
