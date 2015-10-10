#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ORTHOGRAPHIC_CAMERA_H_
#define _SPICA_ORTHOGRAPHIC_CAMERA_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_ORTHOGRAPHIC_CAMERA_EXPORT
        #define SPICA_ORTHOGRAPHIC_CAMERA_DLL __declspec(dllexport)
    #else
        #define SPICA_ORTHOGRAPHIC_CAMERA_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_ORTHOGRAPHIC_CAMERA_DLL
#endif

#include "../math/vector3d.h"
#include "../core/rect.h"

#include "camera_interface.h"

namespace spica {

    /** Orthographic camera.
     * @ingroup camera_module
     */
    class SPICA_ORTHOGRAPHIC_CAMERA_DLL OrthographicCamera : public ICamera {
    private:
        Rect _rect;

    public:
        OrthographicCamera();
        OrthographicCamera(const Vector3D& center,
                         const Vector3D& direction,
                         const Vector3D& up,
                         int imageW, int imageH, double sensitivity);
        ~OrthographicCamera();

        OrthographicCamera(const OrthographicCamera& camera);
        OrthographicCamera& operator=(const OrthographicCamera& camera);

        CameraSample sample(double px, double py, Stack<double>& stk) const override;
        ICamera* clone() const override;

        inline Rect rect() const { return _rect; }
    };

}  // namespace spica

#endif  // _SPICA_ORTHOGRAPHIC_CAMERA_H_
