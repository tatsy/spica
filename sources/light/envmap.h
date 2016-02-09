#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ENVMAP_H_
#define _SPICA_ENVMAP_H_

#include <vector>

#include "light_interface.h"

#include "../core/spectrum.h"
#include "../image/image.h"
#include "../math/vector3d.h"
#include "../shape/sphere.h"

namespace spica {

    class Photon;

    /** Environment mapping
     *  @ingroup light_module
     */
    class SPICA_EXPORTS Envmap : public Light {
    private:
        Sphere _sphere;
        Image _image;
        Image _lowres;
        Image _importance;
        std::vector<double> _pdf;
        std::vector<double> _cdf;
        static const int IMPORTANCE_MAP_SIZE = 64;

    public:
        /** The Envmap constructor. */
        Envmap();

        /** The Envmap constructor.
         *  @param[in] boundSphere: Bounding sphere of the scene used for mapping the enviroment map texture.
         *  @param[in] filename: File name of the HDR image for enviroment map.
         */
        Envmap(const Sphere& boundSphere, const std::string& filename);

        /** The Envmap constructor.
         *  @param[in] boundSphere: Bounding sphere of the scene used for mapping the enviroment map texture.
         *  @param[in] image: HDR image for enviroment map.
         */
        Envmap(const Sphere& boundSphere, const Image& image);

        /** The Envmap constructor (copy).
         */
        Envmap(const Envmap& envmap);

        /** The Envmap constructor (move).
         */
        Envmap(Envmap&& envmap);

        /** The Envmap destructor.
         */
        ~Envmap();

        Envmap& operator=(const Envmap& envmap);
        Envmap& operator=(Envmap&& envmap);

        void resize(int width, int height);

        /** Direct light sampling.
         *  @details This method is usually called for direct light sampling.
         */
        Spectrum directLight(const Vector3D& dir) const override;

        /** Global light sampling.
         *  @details This method is usually called when the ray does not intersect anything.
         */
        Spectrum globalLight(const Vector3D& dir) const override;

        double area() const override;

        LightSample sample(const Point& v, Stack<double>& rands) const override;
        Photon samplePhoton(Stack<double>& rands) const override;

        const Image& getImage() const;

        Light* clone() const override;

    private:
        void createImportanceMap();
        void createLowResolution();
        void sampleOnLight(Point* pos, Vector3D* dir, Normal* nrm, Spectrum* emt, double* pdf, Stack<double>& rands) const;
    };

}  // namespace spica

#endif  // _SPICA_ENVMAP_H_
