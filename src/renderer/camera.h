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
#include "../random/random.h"
#include "../random/halton.h"

#include "ray.h"

namespace spica {

    struct CameraSample;

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
            Vector3D center;         // Center position of the sensor
            Vector3D direction;      // Direction of the sensor
            Vector3D up;             // Up direction of the sensor
            Vector3D unitU;          // Unit vector of u-axis
            Vector3D unitV;          // Unit vector of v-axis
            double sensitivity;     // Sensor sensitivity
        };


        // ------------------------------------------------------------
        // Lens
        // ------------------------------------------------------------

        struct Lens {
            double focalLength;     // Focal length
            double radius;          // Lens radius
            Vector3D center;         // Center position
            Vector3D unitU;          // Unit vector of u-axis
            Vector3D unitV;          // Unit vector of v-axis
            Vector3D normal;         // Lens normal

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
            Vector3D center;     // Center position
            Vector3D normal;     // Normal of the object plane
            Vector3D unitU;      // Unit vector of u-axis
            Vector3D unitV;      // Unit vector of v-axis
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
               const  Vector3D& sensorCenter,
               const  Vector3D& sensorDir,
               const  Vector3D& sensorUp,
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

        double PImageToPAx1(const double PImage, const Vector3D& x0xV, const Vector3D& x0x1, const Vector3D& orientNormal) const;

        /* Return distance to the intersecting point on the lens
         * @param[in] ray: a ray casted to lens
         * @param[out] positionOnLens: hit point on lens
         * @param[out] positionOnObjplane: hit point on object plane
         * @param[out] uvOnSensor: uv coordinate on the sensor
         */
        double intersectLens(const Ray& ray, Vector3D& positionOnLens, Vector3D& positonOnObjplane, Vector3D& positionOnSensor, Vector3D& uvOnSensor) const;

        double contribSensitivity(const Vector3D& x0xV, const Vector3D& x0xI, const Vector3D& x0x1) const;

        void samplePoints(const int imageX, const int imageY, Random& rng, Vector3D& positionOnSensor, Vector3D& positionOnObjplane, Vector3D& positionOnLens, double& PImage, double& PLens) const;

        CameraSample sample(const double imageX, const double imageY, double randnum[4]) const;
        CameraSample sample(const double imageX, const double imageY, RandomSeq& rseq) const;

        inline unsigned int imageW() const { return width_; }
        inline unsigned int imageH() const { return height_; }
        inline void imageW(int width) { width_ = width; }
        inline void imageH(int height) { height_ = height; }
        inline double distSL() const { return distSensorToLens_; }
        
        inline Vector3D center()    const { return sensor_.center; }
        inline Vector3D direction() const { return sensor_.direction; }
        inline Vector3D up()        const { return sensor_.up; }

        inline double  sensorW()   const { return sensor_.width; }
        inline double  sensorH()   const { return sensor_.height; }
        inline Vector3D sensorU()   const { return sensor_.unitU; }
        inline Vector3D sensorV()   const { return sensor_.unitV; }
        inline double  cellW() const { return sensor_.cellW; }
        inline double  cellH() const { return sensor_.cellH; }
        inline double  sensitivity() const { return sensor_.sensitivity; }

        inline Vector3D lensU() const { return lens_.unitU; }
        inline Vector3D lensV() const { return lens_.unitV; }
        inline Vector3D lensCenter() const { return lens_.center; }
        inline Vector3D lensNormal() const { return lens_.normal; }
        inline double  lensRadius() const { return lens_.radius; }
        inline double  lensArea()   const { return lens_.area(); }
        inline double  focalLength() const { return lens_.focalLength; }

        inline Vector3D objplaneCenter() const { return objplane_.center; }
        inline double  objplaneW() const { return objplane_.width; }
        inline double  objplaneH() const { return objplane_.height; }
        inline Vector3D objplaneU() const { return objplane_.unitU; }
        inline Vector3D objplaneV() const { return objplane_.unitV; }
    };

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


}

#endif  // SPICA_CAMERA_H_
