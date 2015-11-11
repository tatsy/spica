/** 
 * @defgroup light_module Light
 * @brief light
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_LIGHTING_H_
#define _SPICA_LIGHTING_H_

#include <memory>
#include <string>

#include "light_interface.h"

#include "../math/vector3d.h"
#include "../core/stack.h"
#include "../core/image.h"
#include "../shape/triangle.h"
#include "../shape/sphere.h"

namespace spica {

    /** Lighting class
     *  @ingroup light_module
     */
    class SPICA_EXPORTS Lighting {
    private:
        std::unique_ptr<ILight> _ptr;

    public:
        /** The Lighting constructor.
         */
        Lighting();

        /** The Lighting constructor (copy).
         */
        Lighting(const Lighting& l);

        /** The Lighting constructor (move)
         */
        Lighting(Lighting&& l);

        /** The lighting destructor.
         */
        ~Lighting();

        Lighting& operator=(const Lighting& l);
        Lighting& operator=(Lighting&& l);

        /** Initialize lighting as environment map (with image file name)
         */
        static Lighting asEnvmap(const Sphere& boundSphere, const std::string& filename);

        /** Initialize lighting as environment map (with image)
         */
        static Lighting asEnvmap(const Sphere& boundSphere, const Image& image);

        /** Initialize lighting as area light
         */
        static Lighting asAreaLight(const Trimesh& triangles, const Color& emittance);

        /** Sample position on light.
         *  @param[in] v: Previous hit point.
         *  @param[in] rands: Random numbers.
         */
        LightSample sample(const Vector3D& v, Stack<double>& rands) const;

        /** Sample photon.
         *  @param[in] rands: Random numbers.
         */
        Photon samplePhoton(Stack<double>& rands) const;

        Color directLight(const Vector3D& dir) const;
        Color globalLight(const Vector3D& dir) const;

        ILight* ptr() const;
        double area() const;
        LightType type() const;
    };

}  // namespace spica

#endif  // _SPICA_LIGHTING_H_
