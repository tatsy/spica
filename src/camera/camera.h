/**
 * @defgroup camera_module Camera
 * @brief camera
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CAMERA_H_
#define _SPICA_CAMERA_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifndef SPICA_CAMERA_EXPORT
        #define SPICA_CAMERA_DLL __declspec(dllexport)
    #else
        #define SPICA_CAMERA_DLL __declspec(dllimport)
    #endif
#else
    #define SPICA_CAMERA_DLL
#endif

#include <memory>

#include "camera_interface.h"
#include "../utils/vector3d.h"
#include "../renderer/ray.h"

namespace spica {

    class CameraSample;

    enum class CameraType {
        Perspective  = 0x01,
        Orthogonal   = 0x02,
        DepthOfField = 0x04
    };

    /** Camera class
     * @ingroup camera_module
     */
    class SPICA_CAMERA_DLL Camera {
    private:
        ICamera*   _ptr;
        CameraType _type;

    public:
        Camera();
        ~Camera();

        Camera(const Camera& camera);
        Camera(Camera&& camera);

        Camera& operator=(const Camera&);
        Camera& operator=(Camera&& camera);

        CameraSample sample(double px, double py, Stack<double>& rstack) const;

        static Camera asDoF(int imageWidth,
                            int imageHeight,
                            const  Vector3D& sensorCenter,
                            const  Vector3D& sensorDir,
                            const  Vector3D& sensorUp,
                            double sensorSize,
                            double distSensorToLens,
                            double focalLength,
                            double lensRadius,
                            double sensorSensitivity);

        static Camera perspective(const Vector3D& center,
                                  const Vector3D& direction,
                                  const Vector3D& up,
                                  double fov, int imageW, int imageH,
                                  double sensitivity);

        static Camera orthogonal(const Vector3D& center,
                                 const Vector3D& direction,
                                 const Vector3D& up,
                                 int imageW, int imageH, double sensitivity);

        std::unique_ptr<ICamera> ptr() const;
        inline CameraType type() const { return _type; }
        inline Vector3D center() const { return _ptr->center(); }
        inline Vector3D direction() const { return _ptr->direction(); }
        inline Vector3D up() const { return _ptr->up(); }
        inline int imageW() const { return _ptr->imageW(); }
        inline int imageH() const { return _ptr->imageH(); }
        inline double sensitivity() const { return _ptr->sensitivity(); }

        // Friend classes
        friend class QGLRenderWidget;
        friend class BDPTRenderer;
    };

    /*
    struct CameraSample {
        // TODO: camera is a pointer and public member, it should be fixed !!
        Vector3D posSensor;
        Vector3D posObjectPlane;
        Vector3D posLens;
        double pdfImage;
        double pdfLens;
        const Camera* camera;

        // Compute ray corresponding to this sample
        Ray generateRay() const {
            return Ray(posLens, Vector3D::normalize(posObjectPlane - posLens));
        }

        // Probability density for this sample
        double totalPdf() const {
            const Vector3D lensToSensor = posSensor - posLens;
            const double cosine = Vector3D::dot(camera->direction(), lensToSensor.normalized());
            const double weight = cosine * cosine / lensToSensor.squaredNorm();
            return pdfLens * pdfImage / weight;
        }
    };
    */

}  // namespace spica

#endif  // _SPICA_CAMERA_H_
