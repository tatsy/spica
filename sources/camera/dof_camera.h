#ifdef _MSC_VER
#pragma once
#endif

#ifndef SPICA_CAMERA_H_
#define SPICA_CAMERA_H_

#include "camera_interface.h"

#include "../image/image.h"
#include "../core/stack.h"

#include "../random/random.h"
#include "../random/halton.h"

#include "../renderer/ray.h"

namespace spica {

    class CameraSample;

    /** Depth of field camera.
     *  @ingroup camera_module
     */
    class SPICA_EXPORTS DoFCamera : public ICamera {
    private:

        // ------------------------------------------------------------
        // Image sensor
        // ------------------------------------------------------------

        struct ImageSensor {
            double width;           // # of sensors along x-axis
            double height;          // # of sensors along y-axis
            double cellW;           // Width of one sensor cell
            double cellH;           // Height of one sensor cell
            Point    center;        // Center position of the sensor
            Vector3D direction;     // Direction of the sensor
            Vector3D up;            // Up direction of the sensor
            Vector3D unitU;         // Unit vector of u-axis
            Vector3D unitV;         // Unit vector of v-axis
            double sensitivity;     // Sensor sensitivity
        };


        // ------------------------------------------------------------
        // Lens
        // ------------------------------------------------------------

        struct Lens {
            double focalLength;     // Focal length
            double radius;          // Lens radius
            Point    center;         // Center position
            Vector3D unitU;          // Unit vector of u-axis
            Vector3D unitV;          // Unit vector of v-axis
            Normal   normal;         // Lens normal

            // Area of lens
            double area() const {
                return PI * radius * radius;
            }
        };


        // ------------------------------------------------------------
        // Object plane
        // ------------------------------------------------------------

        struct ObjectPlane {
            double width;       // Width of the object plane
            double height;      // Height of the object plane
            Point    center;     // Center position
            Normal   normal;     // Normal of the object plane
            Vector3D unitU;      // Unit vector of u-axis
            Vector3D unitV;      // Unit vector of v-axis
        };

    // ------------------------------------------------------------
    // Camera
    // ------------------------------------------------------------

    private:
        // Image size
        double       distSensorToLens_;

        ImageSensor sensor_;
        Lens        lens_;
        ObjectPlane objplane_;

    public:
        DoFCamera();
        DoFCamera(int    imageWidth,
                  int    imageHeight,
                  const  Point& sensorCenter,
                  const  Vector3D& sensorDir,
                  const  Vector3D& sensorUp,
                  double sensorSize,
                  double distSensorToLens,
                  double focalLength,
                  double lensRadius,
                  double sensorSensitivity);

        DoFCamera(const DoFCamera& camera);

        ~DoFCamera();

        DoFCamera& operator=(const DoFCamera& camera);

        ICamera* clone() const override;

        double PImageToPAx1(const double PImage, const Vector3D& x0xV, const Vector3D& x0x1, const Normal& orientNormal) const;

        /* Return distance to the intersecting point on the lens
         * @param[in] ray: a ray casted to lens
         * @param[out] positionOnLens: hit point on lens
         * @param[out] positionOnObjplane: hit point on object plane
         * @param[out] uvOnSensor: uv coordinate on the sensor
         */
        double intersectLens(const Ray& ray, Point* positionOnLens, Point* positonOnObjplane, Point* positionOnSensor, Vector3D* uvOnSensor) const;

        double contribSensitivity(const Vector3D& x0xV, const Vector3D& x0xI, const Vector3D& x0x1) const;

        void samplePoints(const int imageX, const int imageY, Random& rng,
                          Point* positionOnSensor, Point* positionOnObjplane, Point* positionOnLens,
                          double* PImage, double* PLens) const;

        CameraSample sample(double imageX, double imageY, Stack<double>& rseq) const override;

        inline double distSL() const { return distSensorToLens_; }
        
        inline Point    center()    const override { return sensor_.center; }
        inline Vector3D direction() const override { return sensor_.direction; }
        inline Vector3D up()        const override { return sensor_.up; }

        inline double  sensorW()   const { return sensor_.width; }
        inline double  sensorH()   const { return sensor_.height; }
        inline Vector3D sensorU()   const { return sensor_.unitU; }
        inline Vector3D sensorV()   const { return sensor_.unitV; }
        inline double  cellW() const { return sensor_.cellW; }
        inline double  cellH() const { return sensor_.cellH; }
        inline double  sensitivity() const override { return sensor_.sensitivity; }

        inline Vector3D lensU() const { return lens_.unitU; }
        inline Vector3D lensV() const { return lens_.unitV; }
        inline Point    lensCenter() const { return lens_.center; }
        inline Normal   lensNormal() const { return lens_.normal; }
        inline double  lensRadius() const { return lens_.radius; }
        inline double  lensArea()   const { return lens_.area(); }
        inline double  focalLength() const { return lens_.focalLength; }

        inline Point objplaneCenter() const { return objplane_.center; }
        inline double  objplaneW() const { return objplane_.width; }
        inline double  objplaneH() const { return objplane_.height; }
        inline Vector3D objplaneU() const { return objplane_.unitU; }
        inline Vector3D objplaneV() const { return objplane_.unitV; }
    };

}

#endif  // SPICA_CAMERA_H_
