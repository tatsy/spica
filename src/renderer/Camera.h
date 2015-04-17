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

#include "../geometry/Plane.h"
#include "../utils/Image.h"
#include "../utils/Random.h"

namespace spica {

    class Camera;

    // ------------------------------------------------------------
    // Image sensor
    // ------------------------------------------------------------

    class ImageSensor {
        friend class Camera;

    private:
        double _width;
        double _height;
        double _pixelWidth;
        double _pixelHeight;
        Vector3 _center;
        Vector3 _direction;
        Vector3 _up;
        Vector3 _u;
        Vector3 _v;
        double _sensitivity;

    public:
        ImageSensor();
        ImageSensor(double width, double height, Vector3 center, Vector3 direction, Vector3 up, Vector3 u, Vector3 v, double sensitivity);
        ImageSensor(const ImageSensor& sensor);

        ~ImageSensor();

        ImageSensor& operator=(const ImageSensor& sensor);

        inline double width() const { return _width; }
        inline double height() const { return _height; }
        inline Vector3 center() const { return _center; }
        inline Vector3 direction() const { return _direction; }
        inline Vector3 u() const { return _u; }
        inline Vector3 v() const { return _v; }
    };


    // ------------------------------------------------------------
    // Lens
    // ------------------------------------------------------------

    class Lens {
        friend class Camera;

    private:
        double _focalLength;
        double _radius;
        Vector3 _center;
        Vector3 _u;
        Vector3 _v;
        Vector3 _normal;

    public:
        Lens();
        Lens(double focalLength, double radius, Vector3 center, Vector3 u, Vector3 v, Vector3 normal);
        Lens(const Lens& lens);
        
        ~Lens();

        Lens& operator=(const Lens& lens);

        inline Vector3 normal() const { return _normal; }
    };


    // ------------------------------------------------------------
    // Object plane
    // ------------------------------------------------------------

    class ObjectPlane : public Plane {
        friend class Camera;

    private:
        Vector3 _center;
        Vector3 _u;
        Vector3 _v;

    public:
        ObjectPlane();
        ObjectPlane(Vector3 center, Vector3 u, Vector3 v);
        ObjectPlane(const ObjectPlane& objplane);
        ~ObjectPlane();

        ObjectPlane& operator=(const ObjectPlane& objplane);
    };

    // ------------------------------------------------------------
    // Camera
    // ------------------------------------------------------------

    class SPICA_CAMERA_DLL Camera {
    private:
        // Image size
        unsigned int _width;
        unsigned int _height;
        double _distSensorToLens;

        ImageSensor _sensor;
        Lens _lens;
        ObjectPlane _objplane;

    public:
        Camera();
        Camera(int imageWidth, int imageHeight, const Vector3& sensorCenter, const Vector3& sensorDir,  const Vector3& sensorUp, double sensorSize,
               double distSensorToLens, double focalLength, double lensRadius, double sensorSensitivity);
        Camera(const Camera& camera);

        ~Camera();

        Camera& operator=(const Camera& camera);

        double PImageToPAx1(const double PImage, const Vector3& x0xV, const Vector3& x0x1, const Vector3& orientNormal) const;

        bool intersectLens(const Ray& ray, Vector3& positionOnLens, Vector3& positonOnObjplane, Vector3& positionOnSensor, Vector3& uvOnSensor) const;

        double contribSensitivity(const Vector3& x0xV, const Vector3& x0xI, const Vector3& x0x1) const;

        void samplePoints(const int imageX, const int imageY, const Random& rng, Vector3& positionOnSensor, Vector3& positionOnObjplane, Vector3& positionOnLens, double& PImage, double& PLens) const;

        inline unsigned int imageWidth() const { return _width; }
        inline unsigned int imageHeight() const { return _height; }
        
        inline const ImageSensor& sensor() const { return _sensor; }
        inline const Lens& lens() const { return _lens; }
        
    };

}

#endif  // SPICA_CAMERA_H_
