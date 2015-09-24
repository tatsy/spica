#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_DETAIL_H_
#define _SPICA_SCENE_DETAIL_H_

#include "bsdf.h"
#include "brdf.h"

namespace spica {

    template <class T>
    void Scene::addShape(const T& shape, const BSDF& bsdf) {
        static_assert(std::is_base_of<IGeometry, T>::value,
                        "Added shape must derive IGeometry !!");

        std::vector<Triangle> tris = shape.triangulate();

        // Both normal geoms and light geoms are set to triangles
        _triangles.insert(_triangles.end(), tris.begin(), tris.end());
            
        // Update BSDF ids
        const int newBsdfId = static_cast<int>(_bsdfs.size());
        const int numTriangles = static_cast<int>(_bsdfIds.size());
        _bsdfIds.resize(_bsdfIds.size() + tris.size());
        std::fill(_bsdfIds.begin() + numTriangles,
                    _bsdfIds.end(), newBsdfId);
        _bsdfs.push_back(bsdf);    
    }

    template <class T>
    void Scene::setLight(const T& shape, const Color& emittance) {
        static_assert(std::is_base_of<IGeometry, T>::value,
                      "Added shape must derive IGeometry !!");
        
        std::vector<Triangle> tris = shape.triangulate();
        const int newTris = static_cast<int>(tris.size());
        const int nowTris = static_cast<int>(_triangles.size());
        _lighting = Lighting::asAreaLight(tris, emittance);

        // If new object is a light, store triangle indices
        for (int i = 0; i < newTris; i++) {
            _lightIds.push_back(nowTris + i);
        }

        // Both normal geoms and light geoms are set to triangles
        _triangles.insert(_triangles.end(), tris.begin(), tris.end());

        // Add empty BSDF
        const int bsdfID  = static_cast<int>(_bsdfs.size());
        _bsdfIds.resize(_bsdfIds.size() + tris.size());
        std::fill(_bsdfIds.begin() + nowTris, _bsdfIds.end(), bsdfID);
        _bsdfs.push_back(LambertianBRDF::factory(Color(0.0, 0.0, 0.0)));
    }

}  // namespace spica

#endif  // _SPICA_SCENE_DETAIL_H_
