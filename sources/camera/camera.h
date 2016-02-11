/**
 * @defgroup camera_module Camera
 * @brief camera
 */

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _SPICA_CAMERA_H_
#define _SPICA_CAMERA_H_

#include <memory>

#include "../core/forward_decl.h"
#include "camera_interface.h"
#include "../math/vector3d.h"
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
    class SPICA_EXPORTS Camera {
    private:
        std::unique_ptr<ICamera> _ptr;
        CameraType _type;

    public:
        /** The Camera constructor.
         */
        Camera();

        /** The Camera destructor.
         */
        ~Camera();

        /** The Camera constructor (copy).
         */
        Camera(const Camera& camera);

        /** The Camera constructor (move).
         */
        Camera(Camera&& camera);

        Camera& operator=(const Camera&);
        Camera& operator=(Camera&& camera);

        CameraSample sample(double px, double py, const Point2D& rands) const;

        /** Instantiate Camera by DoF camera.
         */
        static Camera asDoF(int imageWidth,
                            int imageHeight,
                            const  Point& sensorCenter,
                            const  Vector3D& sensorDir,
                            const  Vector3D& sensorUp,
                            double sensorSize,
                            double distSensorToLens,
                            double focalLength,
                            double lensRadius,
                            double sensorSensitivity);

        /** Instantiate Camera by perspective camera
         * @param center Camera center position
         * @param direction Camera direction
         * @param up Upward direction
         * @param fov Field of view (degree)
         * @param imageW Image width
         * @param imageH Image height
         * @param sensitivity Camera sensor sensitivity
         */
        static Camera perspective(const Point& center,
                                  const Vector3D& direction,
                                  const Vector3D& up,
                                  double fov, int imageW, int imageH,
                                  double sensitivity);

        /** Instantiate Camera by orthographic camera
         * @param center Camera center position
         * @param direction Camera direction
         * @param up Upward direction
         * @param rect Viewing rectangle
         * @param imageW Image width
         * @param imageH Image height
         * @param sensitivity Camera sensor sensitivity
         */
        static Camera ortho(const Point& center,
                            const Vector3D& direction,
                            const Vector3D& up,
                            const Rect& rect,
                            int imageW, int imageH, double sensitivity);

        /** Simply get the pointer from interior unique_ptr.
         */
        ICamera* getPtr() const;

        /** Get the pointer from interior unique_ptr and release it.
         */
        ICamera* releasePtr();

        inline bool isNull() const { return !static_cast<bool>(_ptr); }

        inline CameraType type() const { return _type; }
        inline Point    center() const { return _ptr->center(); }
        inline Vector3D direction() const { return _ptr->direction(); }
        inline Vector3D up() const { return _ptr->up(); }
        inline int imageW() const { return _ptr->imageW(); }
        inline int imageH() const { return _ptr->imageH(); }
        inline double sensitivity() const { return _ptr->sensitivity(); }

        // Friend classes
        friend class QGLRenderWidget;
        friend class BDPTRenderer;
    };

}  // namespace spica

#endif  // _SPICA_CAMERA_H_
