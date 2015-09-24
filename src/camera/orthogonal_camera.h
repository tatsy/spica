#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_ORTHOGONAL_CAMERA_H_
#define _SPICA_ORTHOGONAL_CAMERA_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_ORTHOGONAL_CAMERA_EXPORT
        #define SPICA_ORTHOGONAL_CAMERA_DLL __declspec(dllexport)
    #else
        #define SPICA_ORTHOGONAL_CAMERA_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_ORTHOGONAL_CAMERA_DLL
#endif

#include "../utils/vector3d.h"
#include "../utils/rect.h"

#include "camera_interface.h"

namespace spica {

    class SPICA_ORTHOGONAL_CAMERA_DLL OrthogonalCamera : public ICamera {
    private:
        Rect _rect;

    public:
        OrthogonalCamera();
        OrthogonalCamera(const Vector3D& center,
                         const Vector3D& direction,
                         const Vector3D& up,
                         int imageW, int imageH, double sensitivity);
        ~OrthogonalCamera();

        OrthogonalCamera(const OrthogonalCamera& camera);
        OrthogonalCamera& operator=(const OrthogonalCamera& camera);

        CameraSample sample(double px, double py, Stack<double>& stk) const override;
        ICamera* clone() const override;

        inline Rect rect() const { return _rect; }
    };

}  // namespace spica

#endif  // _SPICA_ORTHOGONAL_CAMERA_H_
