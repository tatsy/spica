#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENVMAP_H_
#define _SPICA_ENVMAP_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_ENVMAP_EXPORT
        #define SPICA_ENVMAP_DLL __declspec(dllexport)
    #else
        #define SPICA_ENVMAP_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_ENVMAP_DLL
#endif

#include <vector>

#include "light_interface.h"

#include "../utils/vector3d.h"
#include "../utils/color.h"
#include "../utils/image.h"
#include "../geometry/sphere.h"

namespace spica {

    class Photon;

    class SPICA_ENVMAP_DLL Envmap : public ILight {
    private:
        Sphere _sphere;
        Image _image;
        Image _importance;
        std::vector<double> _pdf;
        std::vector<double> _cdf;
        static const int IMPORTANCE_MAP_SIZE = 64;

    public:
        Envmap();
        Envmap(const Sphere& boundSphere, const std::string& filename);

        void resize(int width, int height);
        void clearColor(const Color& color);

        Color directLight(const Vector3D& dir) const override;
        double area() const override;
        LightSample sample(double r1, double r2, double r3) const override;

        Color sampleFromDir(const Vector3D& dir) const;
        const Image& getImage() const;

        ILight* clone() const override;

    private:
        void createImportanceMap();

    };

}

#endif  // _SPICA_ENVMAP_H_
