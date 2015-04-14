#include "Scene.h"

namespace rainy {
    bool intersectScene(const Ray& ray, Intersection& intersection) {
        const int nSphere = sizeof(spheres) / sizeof(Sphere);

        // Linear search
        for (int i = 0; i < nSphere; i++) {
            HitPoint hitpoint;
            if (spheres[i].intersect(ray, hitpoint)) {
                if (hitpoint.distance() < intersection.hittingDistance()) {
                    intersection.setHitPoint(hitpoint);
                    intersection.setObjectId(i);
                }
            }
        }

        return (intersection.objectId() != -1);
    }
}
