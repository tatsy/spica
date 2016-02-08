#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_PERSPECTIVE_CAMERA_H_
#define _SPICA_PERSPECTIVE_CAMERA_H_

#include "../math/vector3d.h"
#include "camera_interface.h"

namespace spica {

    /** Perspective camera
     * @ingroup camera_module
     */
    class SPICA_EXPORTS PerspectiveCamera : public ICamera {
    private:
        double _fov;
        double _aspect;
        double _halfTangent;

    public:
        PerspectiveCamera();
        PerspectiveCamera(const Vector3D& center, const Vector3D& direction,
                          const Vector3D& up, double fov,
                          int imageW, int imageH, double sensitivity);
        PerspectiveCamera(const PerspectiveCamera& camera);
        ~PerspectiveCamera();

        PerspectiveCamera& operator=(const PerspectiveCamera& camera);

        CameraSample sample(double px, double py, Stack<double>& stk) const override;
        ICamera* clone() const override;

        inline double fov() const { return _fov; }
        inline double aspect() const { return _aspect; }
    };

}  // namespace spica

#endif  // _SPICA_PERSPECTIVE_CAMERA_H_
