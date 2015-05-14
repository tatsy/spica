#ifndef SPICA_CAMERA_H_
#define SPICA_CAMERA_H_

#if defined(_WIN32) || defined(__WIN32__)
    #ifdef SPICA_CAMERA_EXPORT
        #define SPICA_CAMERA_DLL __declspec(dllexport)
    #else
        #define SPICA_CAMERA_DLL __declspec(dllimport)
    #endif
#elif defined(linux) || defined(__linux)
    #define SPICA_CAMERA_DLL
#endif

#include "../geometry/plane.h"
#include "../utils/image.h"
#include "../utils/random.h"

namespace spica {

    class SPICA_CAMERA_DLL Camera {
    private:

        // ------------------------------------------------------------
        // Image sensor
        // ------------------------------------------------------------

        struct ImageSensor {
            double width;           // # of sensors along x-axis
            double height;          // # of sensors along y-axis
            double cellW;           // Width of one sensor cell
            double cellH;           // Height of one sensor cell
            Vector3 center;         // Center position of the sensor
            Vector3 direction;      // Direction of the sensor
            Vector3 up;             // Up direction of the sensor
            Vector3 unitU;          // Unit vector of u-axis
            Vector3 unitV;          // Unit vector of v-axis
            double sensitivity;     // Sensor sensitivity
        };


        // ------------------------------------------------------------
        // Lens
        // ------------------------------------------------------------

        struct Lens {
            double focalLength;     // Focal length
            double radius;          // Lens radius
            Vector3 center;         // Center position
            Vector3 unitU;          // Unit vector of u-axis
            Vector3 unitV;          // Unit vector of v-axis
            Vector3 normal;         // Lens normal

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
            Vector3 center;     // Center position
            Vector3 normal;     // Normal of the object plane
            Vector3 unitU;      // Unit vector of u-axis
            Vector3 unitV;      // Unit vector of v-axis
        };

    // ------------------------------------------------------------
    // Camera
    // ------------------------------------------------------------

    private:
        // Image size
        unsigned int width_;                // size of image
        unsigned int height_;               // size of height
        double       distSensorToLens_;

        ImageSensor sensor_;
        Lens        lens_;
        ObjectPlane objplane_;

    public:
        Camera();
        Camera(int    imageWidth,
               int    imageHeight,
               const  Vector3& sensorCenter,
               const  Vector3& sensorDir,
               const  Vector3& sensorUp,
               double sensorSize,
               double distSensorToLens,
               double focalLength,
               double lensRadius,
               double sensorSensitivity);

        Camera(const Camera& camera);

        ~Camera();

        Camera& operator=(const Camera& camera);

        /* Compute a ray from 
         */
        Ray rayToObjectPlane(double x, double y) const;

        double PImageToPAx1(const double PImage, const Vector3& x0xV, const Vector3& x0x1, const Vector3& orientNormal) const;

        /* Return distance to the intersecting point on the lens
         * @param[in] ray: a ray casted to lens
         * @param[out] positionOnLens: hit point on lens
         * @param[out] positionOnObjplane: hit point on object plane
         * @param[out] uvOnSensor: uv coordinate on the sensor
         */
        double intersectLens(const Ray& ray, Vector3& positionOnLens, Vector3& positonOnObjplane, Vector3& positionOnSensor, Vector3& uvOnSensor) const;

        double contribSensitivity(const Vector3& x0xV, const Vector3& x0xI, const Vector3& x0x1) const;

        void samplePoints(const int imageX, const int imageY, const Random& rng, Vector3& positionOnSensor, Vector3& positionOnObjplane, Vector3& positionOnLens, double& PImage, double& PLens) const;

        inline unsigned int imageW() const { return width_; }
        inline unsigned int imageH() const { return height_; }
        inline void imageW(int width) { width_ = width; }
        inline void imageH(int height) { height_ = height; }
        inline double distSL() const { return distSensorToLens_; }
        
        inline Vector3 center()    const { return sensor_.center; }
        inline Vector3 direction() const { return sensor_.direction; }
        inline Vector3 up()        const { return sensor_.up; }

        inline double  sensorW()   const { return sensor_.width; }
        inline double  sensorH()   const { return sensor_.height; }
        inline Vector3 sensorU()   const { return sensor_.unitU; }
        inline Vector3 sensorV()   const { return sensor_.unitV; }

        inline Vector3 lensU() const { return lens_.unitU; }
        inline Vector3 lensV() const { return lens_.unitV; }
        inline Vector3 lensCenter() const { return lens_.center; }
        inline Vector3 lensNormal() const { return lens_.normal; }
        inline double  lensRadius() const { return lens_.radius; }
        inline double  lensArea()   const { return lens_.area(); }
        inline double  focalLength() const { return lens_.focalLength; }

        inline Vector3 objplaneCenter() const { return objplane_.center; }
        inline double  objplaneW() const { return objplane_.width; }
        inline double  objplaneH() const { return objplane_.height; }
        inline Vector3 objplaneU() const { return objplane_.unitU; }
        inline Vector3 objplaneV() const { return objplane_.unitV; }
    };

}

#endif  // SPICA_CAMERA_H_
