#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENVMAP_H_
#define _SPICA_ENVMAP_H_

#include <vector>

#include "light_interface.h"

#include "../core/color.h"
#include "../core/image.h"
#include "../math/vector3d.h"
#include "../shape/sphere.h"

namespace spica {

    class Photon;

    /** Environment mapping
     *  @ingroup light_module
     */
    class SPICA_EXPORTS Envmap : public ILight {
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
        Envmap(const Sphere& boundSphere, const Image& image);

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

}  // namespace spica

#endif  // _SPICA_ENVMAP_H_
