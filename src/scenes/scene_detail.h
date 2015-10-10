#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_SCENE_DETAIL_H_
#define _SPICA_SCENE_DETAIL_H_

#include "../bsdf/bsdf.h"
#include "../bsdf/brdf.h"

extern void* enabler;

namespace spica {

    template <class B, class T>
    using is_base_of_t = typename std::enable_if<std::is_base_of<B, T>::value>::type;

    template <class T, is_base_of_t<IGeometry, T> *& = enabler>
    void Scene::addShape(const T& shape, const BSDF& bsdf) {
        // Copy triangles
        std::vector<Triangle> tris = shape.triangulate();
        addTriangles(tris);

        // Add BSDF
        addBsdf(bsdf, tris.size());
    }

    template <>
    inline void Scene::addShape(const Trimesh& shape, const BSDF& bsdf) {
        // Copy triangles
        std::vector<Triplet> trip = shape.getIndices();
        for (int i = 0; i < trip.size(); i++) {
            const int vid = _vertices.size();
            for (int k = 0; k < 3; k++) {
                _vertices.push_back(shape.getVertexData(trip[i][k]));
            }
            _triangles.emplace_back(vid, vid + 1, vid + 2, shape.isTextured());
        }

        _texture = shape._texture;
                    
        // Update BSDF ids
        addBsdf(bsdf, trip.size());
    }

    template <class T, is_base_of_t<IGeometry, T> *& = enabler>
    void Scene::setLight(const T& shape, const Color& emittance) {
        std::vector<Triangle> tris = shape.triangulate();
        addTriangles(tris);

        // Set as area light
        _lighting = Lighting::asAreaLight(tris, emittance);

        // If new object is a light, store triangle indices
        const int newTris = static_cast<int>(tris.size());
        const int nowTris = static_cast<int>(_triangles.size()) - newTris;
        for (int i = 0; i < newTris; i++) {
            _lightIds.push_back(nowTris + i);
        }

        // Add empty BSDF
        addBsdf(LambertianBRDF::factory(Color(0.0, 0.0, 0.0)), newTris);
    }

}  // namespace spica

#endif  // _SPICA_SCENE_DETAIL_H_
