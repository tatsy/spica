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
         *  @param[in] r1: First random number.
         *  @param[in] r2: Second random number.
         *  @param[in] r3: Third random number.
         */
        LightSample sample(double r1, double r2, double r3) const;

        Color directLight(const Vector3D& dir) const;
        double area() const;
        LightType type() const;
    };

}  // namespace spica

#endif  // _SPICA_LIGHTING_H_
