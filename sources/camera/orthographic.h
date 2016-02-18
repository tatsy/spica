#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ORTHOGRAPHIC_CAMERA_H_
#define _SPICA_ORTHOGRAPHIC_CAMERA_H_

#include "../math/vector3d.h"
#include "../core/rect.h"

#include "camera_interface.h"

namespace spica {

    /** Orthographic camera.
     *  @ingroup camera_module
     */
    class SPICA_EXPORTS OrthographicCamera : public ICamera {
    private:
        Rect _rect;

    public:
        OrthographicCamera();
        OrthographicCamera(const Point& center,
                         const Vector3D& direction,
                         const Vector3D& up,
                         const Rect& rect,
                         int imageW, int imageH, double sensitivity);
        ~OrthographicCamera();

        OrthographicCamera(const OrthographicCamera& camera);
        OrthographicCamera& operator=(const OrthographicCamera& camera);

        CameraSample sample(double px, double py, const Point2D& rands) const override;
        ICamera* clone() const override;

        inline Rect rect() const { return _rect; }
    };

}  // namespace spica

#endif  // _SPICA_ORTHOGRAPHIC_CAMERA_H_
