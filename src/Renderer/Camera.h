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

#include "../utils/Image.h"

namespace spica {

    // ------------------------------------------------------------
    // Image sensor
    // ------------------------------------------------------------

    class SPICA_CAMERA_DLL ImageSensor {
    private:
        double _width;
        double _height;
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
    };


    // ------------------------------------------------------------
    // Camera
    // ------------------------------------------------------------

    class Camera {
    private:
        // Image size
        int _width;
        int _height;

        // Image sensor parameters
        ImageSensor _sensor;

        // Lens parameters
        Lens _lens;

        // Object plane
        Vector3 _objPlaneCenter;
        Vector3 _objPlaneU;
        Vector3 _objPlaneV;
    };

}

#endif  // SPICA_CAMERA_H_
