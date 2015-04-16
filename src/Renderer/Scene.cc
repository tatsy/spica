#define SPICA_SCENE_EXPORT
#include "Scene.h"

#include <cstring>

namespace spica {
    
    Scene::Scene()
        : _nPrimitives(0)
        , _arraySize(1024)
        , _primitives(0)
    {
        _primitives = new Primitive*[_arraySize];
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

    void Scene::addPlane(const Plane& plane) {
        _primitives[_nPrimitives++] = new Plane(plane);
        checkArraySize();
    }

    void Scene::addSphere(const Sphere& sphere) {
        _primitives[_nPrimitives++] = new Sphere(sphere);
        checkArraySize();
    }

    const Primitive* Scene::getObjectPtr(int id) const {
        msg_assert(id >= 0 && id < _nPrimitives, "Object index out of bounds");
        return _primitives[id];
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
